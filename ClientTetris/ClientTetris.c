#include "ClientTetris.h"

#include <pwd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <time.h>

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

typedef struct
{
  TOPSCORE  TopScore;
  int       nbJoueursConnectes;
} MEM;

/////////////////////////////////////////////////////////////////////////////////////:
int ConnectionServeur(key_t cle,const char* pseudo)
{
  int idQ,idUser;
  struct passwd PassWord;
  REQUETE Requete;
  REPONSE Reponse;
  struct timespec delai;

  delai.tv_sec = 1;
  delai.tv_nsec = 500000000;

  if ((idQ = msgget(cle,0)) == -1)
    return -1;  // Serveur non lance

  Requete.type = 1;
  Requete.requete = LOGIN;
  Requete.client = getpid();
  strncpy(Requete.pseudo,pseudo,79);
  idUser = getuid();
  memcpy(&PassWord,getpwuid(idUser),sizeof(struct passwd));
  strncpy(Requete.login,PassWord.pw_name,19);

  if (msgsnd(idQ,&Requete,sizeof(REQUETE)-sizeof(long),0) == -1)
    return -2;

  nanosleep(&delai,NULL); // delai laisse au serveur pour repondre 

  if(msgrcv(idQ,&Reponse,sizeof(REPONSE)-sizeof(long),getpid(),IPC_NOWAIT) == -1)
    return -3;

  if (!Reponse.ok) 
    return -4;

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////:
int EnvoiScore(key_t cle,int score)
{
  int idQ;
  REQUETE Requete;
  REPONSE Reponse;
  struct timespec delai;

  delai.tv_sec = 1;
  delai.tv_nsec = 500000000;

  if ((idQ = msgget(cle,0)) == -1)
    return -1;  // Serveur non lance

  Requete.type = 1;
  Requete.requete = SCORE;
  Requete.client = getpid();
  Requete.score = score;

  if (msgsnd(idQ,&Requete,sizeof(REQUETE)-sizeof(long),0) == -1)
    return -2;

  nanosleep(&delai,NULL); // delai laisse au serveur pour repondre 

  if(msgrcv(idQ,&Reponse,sizeof(REPONSE)-sizeof(long),getpid(),IPC_NOWAIT) == -1)
    return -3;

  if (!Reponse.ok) 
    return -4;

  return Reponse.topScoreBattu;
}

/////////////////////////////////////////////////////////////////////////////////////:
int DeconnectionServeur(key_t cle)
{
  int idQ;
  REQUETE Requete;
  REPONSE Reponse;
  struct timespec delai;

  delai.tv_sec = 1;
  delai.tv_nsec = 500000000;

  if ((idQ = msgget(cle,0)) == -1)
    return -1;  // Serveur non lance

  Requete.type = 1;
  Requete.requete = LOGOUT;
  Requete.client = getpid();

  if (msgsnd(idQ,&Requete,sizeof(REQUETE)-sizeof(long),0) == -1)
    return -2;

  nanosleep(&delai,NULL); // delai laisse au serveur pour repondre 

  if(msgrcv(idQ,&Reponse,sizeof(REPONSE)-sizeof(long),getpid(),IPC_NOWAIT) == -1)
    return -3;

  if (!Reponse.ok) 
    return -4;

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////:
int GetNbJoueursConnectes(key_t cle)
{
  int idM;
  MEM *pShm;
  int nbJoueursConnectes;

  // Recuperation de l'id de la memoire partagee
  if ((idM = shmget(cle,0,0)) == -1)
    return -1;
  
  // Attachement
  if ((pShm = (MEM*)shmat(idM,0,0)) == (MEM *)-1)
    return -2;

  nbJoueursConnectes = pShm->nbJoueursConnectes;
  
  // Detachement
  if(shmdt(pShm))
    return -3;

  return nbJoueursConnectes;
}

/////////////////////////////////////////////////////////////////////////////////////:
int GetTopScore(key_t cle,TOPSCORE *pTopScore)
{
  int idM;
  MEM *pShm;

  // Recuperation de l'id de la memoire partagee
  if ((idM = shmget(cle,0,0)) == -1)
    return -1;
  
  // Attachement
  if ((pShm = (MEM*)shmat(idM,0,0)) == (MEM *)-1)
    return -2;

  memcpy(pTopScore,&(pShm->TopScore),sizeof(TOPSCORE));
  
  // Detachement
  if(shmdt(pShm))
    return -3;

  return 0;
}
