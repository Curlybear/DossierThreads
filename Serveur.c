#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <pthread.h>
#include "Ecran.h"

typedef struct connection
{
  pid_t client;
  char  login[20];
  char  pseudo[80];
  struct connection *suivant;
} CONNECTION;

typedef struct
{
  char  login[20];
  char  pseudo[80];
  int   score;  
} TOPSCORE;

typedef struct
{
  TOPSCORE  TopScore;
  int       nbJoueursConnectes;
} MEM;

#define NOM_FICHIER_TOPSCORE "TopScore.dat"
#define NOM_FICHIER_TRACE    "Serveur.log"

#define LOGIN  1
#define LOGOUT 2
#define SCORE  3
typedef struct
{
  long  type;
  pid_t client;
  int   requete;
  char  login[20];
  char  pseudo[80];
  int   score;
} REQUETE;

typedef struct
{
  long type;
  char ok;
  char topScoreBattu;
} REPONSE;

pthread_mutex_t   mutexConnections;

CONNECTION        *pTete = NULL;           // liste chainee des clients connectes, protege par mutexConnections
int               nbClients = 0;           // nombre de clients connectes, protege par mutexConnections

MEM               *pShm = NULL;    

int               idQ;
int               idM;

int               ajouteClient(pid_t client,const char* login,const char* pseudo);
CONNECTION*       estConnecte(pid_t pid);
int               supprimeClient(pid_t client);
void              AfficheClientsConnectes();
void              keepConnectionsAlive();
void              CleanConnections();
void              EnvoiSignal(int signal);
int               LoadFichierTopScore(const char *nomFichier,TOPSCORE *pTopScore);
int               SaveFichierTopScore(const char *nomFichier,TOPSCORE *pTopScore);

void*             FctThreadKeepAlive(void *p);
pthread_t         threadKeepAlive;

void              SortieErreur(const char* message,int val);

void              HandlerSIGINT(int s);

/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc,char *argv[])
{
  sigset_t  mask;
  struct sigaction sigAct;
  key_t cle;
  REQUETE Requete;
  REPONSE Reponse;
  TOPSCORE TopScore;
  CONNECTION *pC;
  int hd;
  FILE *f;

  if (argc != 2)
  {
    printf("Nombre d'argument insuffisant !\n");
    printf("Usage : Serveur cle\n");
    fflush(stdout);
    exit(1);
  }

  // Redirection de la sortie d'erreur vers Trace.log
  if ((f = fopen(NOM_FICHIER_TRACE,"w")) == NULL)
  {
    perror("Erreur de fopen()...");
    exit(1);
  }
  hd = fileno(f);
  if (dup2(hd,2) != 2)
  {
    perror("Erreur de dup2()...");
    exit(1);
  }

  // Recuperation de la cle des IPC
  cle = atoi(argv[1]);
  Trace("Cle IPC = %d",cle);
  fflush(stdout);

  // Armement et masquage des signaux
  sigfillset(&mask);
  sigdelset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  sigAct.sa_handler = HandlerSIGINT;
  sigAct.sa_flags = 0;
  sigemptyset(&sigAct.sa_mask);
  sigaction(SIGINT,&sigAct,NULL); 

  // Initialisation des mutex
  pthread_mutex_init(&mutexConnections,NULL);

  // Chargement du fichier des score;
  Trace("Chargement Fichier Top Score");
  if (LoadFichierTopScore(NOM_FICHIER_TOPSCORE,&TopScore) < 0)
  {
    perror("Erreur du chargement du fichier Top Score");
    exit(1);
  }

  // Creation de la memoire partagee, attachement et initialisation
  Trace("Creation et initialisation des IPC");
  if ((idM = shmget(cle,sizeof(MEM),IPC_CREAT | IPC_EXCL | 0600)) == -1)
  {
    perror("Erreur de creation de memoire partagee");
    exit(1);
  }

  if ((pShm = (MEM*)shmat(idM,0,0)) == (MEM *)-1)
  {
    perror("Erreur d'attachement a la memoire partagee");
    if (shmctl(idM,IPC_RMID,0)) perror("Erreur de suppression de la memoire partagee");
    exit(1);
  }

  pShm->nbJoueursConnectes = nbClients; // qui est egal a 0
  memcpy(&(pShm->TopScore),&TopScore,sizeof(TOPSCORE));

  // Creation de la file de message
  if ((idQ = msgget(cle,IPC_CREAT | IPC_EXCL | 0600)) == -1)
  {
    perror("Erreur de creation de la file de messages");
    if (shmctl(idM,IPC_RMID,0)) perror("Erreur de suppression de la memoire partagee");
    exit(1);
  } 

  // Lancement du threadKeepAlive
  pthread_create(&threadKeepAlive,NULL,FctThreadKeepAlive,NULL);

  Trace("En attente de requetes...");
  while(1)
  {
    if(msgrcv(idQ,&Requete,sizeof(REQUETE)-sizeof(long),1,0)==-1)
      SortieErreur("Erreur de msgrcv",1);

    Reponse.ok = 0;
    switch(Requete.requete)
    {
      case LOGIN : Trace("Connection Client : pid=%ld login=%s pseudo=%s",Requete.client,Requete.login,Requete.pseudo);
                   if (ajouteClient(Requete.client,Requete.login,Requete.pseudo) == 0) Reponse.ok = 1;
                   else Reponse.ok = 0;
                   break;

      case SCORE :  Trace("Reception Score : pid=%ld score=%d",Requete.client,Requete.score);
                    if (Requete.score > TopScore.score)
                    {
                      // Nouveau TopScore
                      pthread_mutex_lock(&mutexConnections);
                      if ((pC = estConnecte(Requete.client)) != NULL)
                      {
                        Trace("Nouveau Top Score : pid=%ld login=%s pseudo=%s score=%d",Requete.client,pC->login,pC->pseudo,Requete.score);
                        TopScore.score = Requete.score;
                        strcpy(TopScore.login,pC->login);
                        strcpy(TopScore.pseudo,pC->pseudo);

                        if(SaveFichierTopScore(NOM_FICHIER_TOPSCORE,&TopScore) < 0)
                          SortieErreur("Erreur de sauvegarde du fichier Top Score",1);

                        memcpy(&(pShm->TopScore),&TopScore,sizeof(TOPSCORE));

                        Reponse.topScoreBattu = 1;
                      }
                      pthread_mutex_unlock(&mutexConnections);
                      EnvoiSignal(SIGQUIT);
                    }
                    else Reponse.topScoreBattu = 0;
                    Reponse.ok = 1;
                    break;

      case LOGOUT : Trace("Deconnection Client : pid=%ld",Requete.client);
                    fflush(stdout);
                    if (supprimeClient(Requete.client) == 0) Reponse.ok = 1;
                    else Reponse.ok = 0;
                    break;

      default :    break;
    }

    EnvoiSignal(SIGHUP);

    Reponse.type = Requete.client;
    if (msgsnd(idQ,&Reponse,sizeof(REPONSE)-sizeof(long),0) == -1)
      SortieErreur("Erreur de msgsnd",1);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int ajouteClient(pid_t client,const char* login,const char* pseudo)
{
  pthread_mutex_lock(&mutexConnections);
  if (estConnecte(client))
  {
    pthread_mutex_unlock(&mutexConnections);
    return -1;
  }

  CONNECTION *connection = (CONNECTION*)malloc(sizeof(CONNECTION));
  connection->client = client;
  strncpy(connection->login,login,19);
  strncpy(connection->pseudo,pseudo,79);

  connection->suivant = pTete;
  pTete = connection;

  nbClients++;
  Trace("Clients connectes = %d",nbClients);
  pShm->nbJoueursConnectes = nbClients;
  pthread_mutex_unlock(&mutexConnections);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int supprimeClient(pid_t client)
{
  CONNECTION *p,*p2;

  pthread_mutex_lock(&mutexConnections);
  if ((p = estConnecte(client)) == NULL)
  {
    pthread_mutex_unlock(&mutexConnections);
    return -1;
  }

  if (p == pTete) pTete = pTete->suivant;
  else
  {
    p2 = pTete;
    while (p2->suivant != p) p2 = p2->suivant;
    p2->suivant = p->suivant;
  }
  free(p);
  nbClients--;
  Trace("Clients connectes = %d",nbClients);
  pShm->nbJoueursConnectes = nbClients;
  pthread_mutex_unlock(&mutexConnections);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CONNECTION* estConnecte(pid_t pid)
{
  char trouve = 0;
  CONNECTION *p;

//  pthread_mutex_lock(&mutexConnections);
  p = pTete;
  while (p != NULL && !trouve)
  {
    if (p->client == pid) trouve = 1;
    else p = p->suivant;
  }
//  pthread_mutex_unlock(&mutexConnections);
  return p;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void keepConnectionsAlive()
{
  CONNECTION *p,*pT = NULL;
  int  oldNbClients;
  char nbClientsChange = 0;

  pthread_mutex_lock(&mutexConnections);
  oldNbClients = nbClients;
  nbClients = 0;
  while(pTete != NULL)
  {
    p = pTete;
    pTete = pTete->suivant;
   
    if (kill(p->client,0) == 0)
    {
      p->suivant = pT;
      pT = p;
      nbClients++;
    }
    else 
    {
      Trace("Nettoyage Client pid=%d",p->client);
      free(p);
    }
  }
  pTete = pT;
  pShm->nbJoueursConnectes = nbClients;
  if (oldNbClients != nbClients) nbClientsChange = 1; 
  pthread_mutex_unlock(&mutexConnections);

  if (nbClientsChange) 
  {
    Trace("Clients connectes = %d",nbClients); fflush(stdout);
    EnvoiSignal(SIGHUP);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void AfficheClientsConnectes()
{
  CONNECTION *p;

  pthread_mutex_lock(&mutexConnections);
  p = pTete;
  while(p != NULL)
  {
    Trace("Pid=%ld Login=%s Pseudo=%s",p->client,p->login,p->pseudo);
    p = p->suivant;
  }
  pthread_mutex_unlock(&mutexConnections);  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void EnvoiSignal(int signal)
{
  CONNECTION *p,*pT = NULL;
  int  oldNbClients;

  pthread_mutex_lock(&mutexConnections);
  oldNbClients = nbClients;
  nbClients = 0;
  while(pTete != NULL)
  {
    p = pTete;
    pTete = pTete->suivant;
   
    if (kill(p->client,signal) == 0)
    {
      p->suivant = pT;
      pT = p;
      nbClients++;
    }
    else 
    {
      Trace("Nettoyage Client pid=%d",p->client);
      free(p);
    }
  }
  pTete = pT;
  pShm->nbJoueursConnectes = nbClients;
  if (oldNbClients != nbClients) Trace("Clients connectes = %d",nbClients); fflush(stdout);
  pthread_mutex_unlock(&mutexConnections);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CleanConnections()
{
  CONNECTION *p;

  pthread_mutex_lock(&mutexConnections);
  while (pTete != NULL)
  {
    p = pTete;
    pTete = pTete->suivant;
    free(pTete);
  }
  nbClients = 0;
  pthread_mutex_unlock(&mutexConnections);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void HandlerSIGINT(int s)
{
  Trace("Suppression des ressources");
  fflush(stdout);
  CleanConnections();
  pthread_mutex_destroy(&mutexConnections);

  Trace("Suppression des IPC");
  if (msgctl(idQ,IPC_RMID,0))
  {
    perror("Erreur de suppression de la file de messages");
    exit(1);
  }
  
  if(shmdt(pShm))
  {
    perror("Erreur de detachement de la memoire partagee");
    exit(1);
  }

  if (shmctl(idM,IPC_RMID,0)) 
  {
    perror("Erreur de suppression de la memoire partagee");
    exit(1);
  }

  Trace("Fermeture du serveur");
  exit(0); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void SortieErreur(const char* message,int val)
{
  perror(message);
  Trace("Suppression des IPC");

  if (msgctl(idQ,IPC_RMID,0))
  {
    perror("Erreur de suppression de la file de messages");
    exit(1);
  }
  
  if(shmdt(pShm))
  {
    perror("Erreur de detachement de la memoire partagee");
    exit(1);
  }

  if (shmctl(idM,IPC_RMID,0)) 
  {
    perror("Erreur de suppression de la memoire partagee");
    exit(1);
  }
 
  exit(val);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int LoadFichierTopScore(const char *nomFichier,TOPSCORE *pTopScore)
{
  int hd;

  if ((hd = open(nomFichier,O_RDONLY)) == -1)
  {
    // Fichier inexistant, on le bidonne
    if ((hd = open(nomFichier,O_WRONLY|O_CREAT,0700)) == -1)
      return -1;

    strcpy(pTopScore->login,"wagner");
    strcpy(pTopScore->pseudo,"wagner");
    pTopScore->score = 0;

    if (write(hd,pTopScore,sizeof(TOPSCORE)) != sizeof(TOPSCORE))
    {
      close(hd);
      return -1;
    }
    close(hd);
    return 0;
  }

  // Le fichier existe et est ouvert
  if (read(hd,pTopScore,sizeof(TOPSCORE)) != sizeof(TOPSCORE))
  {
    close(hd);
    return -1;
  }

  return 0; 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int SaveFichierTopScore(const char *nomFichier,TOPSCORE *pTopScore)
{
  int hd;

  if ((hd = open(nomFichier,O_WRONLY|O_CREAT,0700)) == -1)
    return -1;

  if (write(hd,pTopScore,sizeof(TOPSCORE)) != sizeof(TOPSCORE))
  {
    close(hd);
    return -1;
  }
  close(hd);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void* FctThreadKeepAlive(void *p)
{
  struct timespec delai;

  delai.tv_sec = 10;
  delai.tv_nsec = 0;

  Trace("Demarrage du ThreadKeepAlive...");
  fflush(stdout);

  while(1)
  {
    nanosleep(&delai,NULL);
    keepConnectionsAlive();
  }
}
