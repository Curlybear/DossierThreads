#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "GrilleSDL.h"
#include "Ressources.h"
#include "ClientTetris.h"

// Dimensions de la grille de jeu
#define NB_LIGNES   14
#define NB_COLONNES 20

// Nombre de cases maximum par piece
#define NB_CASES    4

// Macros utlisees dans le tableau tab
#define VIDE        0

int tab[NB_LIGNES][NB_COLONNES]
 ={ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

typedef struct {
    int ligne;
    int colonne;
} CASE;

typedef struct {
    CASE cases[NB_CASES];
    int  nbCases;
    int  professeur;
} PIECE;

PIECE pieces[7] = { 0, 0, 0, 1, 1, 0, 1, 1, 4, WAGNER,    // carre
                    0, 0, 1, 0, 2, 0, 2, 1, 4, MERCENIER, // L
                    0, 1, 1, 1, 2, 0, 2, 1, 4, VILVENS,   // J
                    0, 0, 0, 1, 1, 1, 1, 2, 4, DEFOOZ,    // Z
                    0, 1, 0, 2, 1, 0, 1, 1, 4, GERARD,    // S
                    0, 0, 0, 1, 0, 2, 1, 1, 4, CHARLET,   // T
                    0, 0, 0, 1, 0, 2, 0, 3, 4, MADANI };  // I

///////////////////////////////////////////////////////////////////////////////////////////////////
char *message = NULL;          // pointeur vers le message à faire défiler
int   tailleMessage;           // longueur du message
int   indiceCourant;           // indice du premier caractère à afficher dans la zone graphique
PIECE pieceEnCours;            // Pièce en cours de placement
CASE  casesInserees[NB_CASES]; // cases insérées par le joueur
int   nbCasesInserees = 0;     // nombre de cases actuellement insérées par le joueur.
char  majScore = 0;
int   score = 0;
int lignesCompletes[4];
int nbLignesCompletes;
int colonnesCompletes[4];
int nbColonnesCompletes;
int nbAnalyses;

pthread_t tabThreadCaseHandle[14][10];

pthread_key_t keyCase;

pthread_mutex_t mutexTab;
pthread_mutex_t mutexMessage; // Mutex pour message, tailleMessage et indiceCourant
pthread_mutex_t mutexPiecesEnCours;
pthread_mutex_t mutexScore;
pthread_mutex_t mutexMaCase;
pthread_mutex_t mutexAnalyse;

pthread_cond_t condNbPiecesEnCours;
pthread_cond_t condScore;

///////////////////////////////////////////////////////////////////////////////////////////////////
void  setMessage(const char*);
char  getCharFromMessage(int);
void  rotationPiece(PIECE*);
int   compareCase(const void*, const void*);
int   random(int, int);
int   comparaisonPiece(CASE[], CASE[], int);
void  setPiece(CASE[], int, int);

void suppressionCase(void*);

// THREADS
void *threadDefileMessage(void*);
void *threadPiece(void*);
void *threadEvent(void*);
void *threadScore(void*);
void *threadCase(void*);

// HANDLERS SIGS

void handlerSIGUSR1(int sig);


///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
    int i,j;
    char buffer[80];
    char ok = 0;
    CASE maCase;

    pthread_t defileMessageHandle;
    pthread_t pieceHandle;
    pthread_t eventHandle;
    pthread_t scoreHandle;

    srand((unsigned)time(NULL));

    pthread_mutex_init(&mutexTab, NULL);
    pthread_mutex_init(&mutexMessage, NULL);
    pthread_mutex_init(&mutexPiecesEnCours, NULL);
    pthread_mutex_init(&mutexScore, NULL);
    pthread_mutex_init(&mutexMaCase, NULL);
    pthread_mutex_init(&mutexAnalyse, NULL);

    pthread_cond_init(&condNbPiecesEnCours, NULL);
    pthread_cond_init(&condScore, NULL);

    struct sigaction sigAct;
    sigset_t mask;


    // Ouverture de la grille de jeu (SDL)
    printf("(THREAD MAIN) Ouverture de la grille de jeu\n");
    fflush(stdout);
    sprintf(buffer, "!!! TETRIS ZERO GRAVITY !!!");
    if (OuvrirGrilleSDL(NB_LIGNES, NB_COLONNES, 40, buffer) < 0) {
        printf("Erreur de OuvrirGrilleSDL\n");
        fflush(stdout);
        exit(1);
    }

    // Chargement des sprites et de l'image de fond
    ChargementImages();
    DessineSprite(12, 11, VOYANT_VERT);

    sigemptyset(&sigAct.sa_mask);
    sigemptyset(&mask);
    sigAct.sa_handler=handlerSIGUSR1;
    sigAct.sa_flags = 0;
    sigaction(SIGUSR1, &sigAct, NULL);

    for (int i = 0; i < 14; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            pthread_mutex_lock(&mutexMaCase);

            maCase.ligne=i;
            maCase.colonne=j;

            if((errno = pthread_create(&tabThreadCaseHandle[i][j], NULL, threadCase, &maCase)) != 0) {
                perror("Erreur de lancement du threadDefileMessage");
            }
        }
    }

    sigaddset(&mask,SIGUSR1);
    pthread_sigmask(SIG_BLOCK,&mask,NULL);

    if((errno = pthread_create(&defileMessageHandle, NULL, threadDefileMessage, NULL)) != 0) {
        perror("Erreur de lancement du threadDefileMessage");
    }
    pthread_detach(defileMessageHandle);
    if((errno = pthread_create(&pieceHandle, NULL, threadPiece, NULL)) != 0) {
       perror("Erreur de lancement du threadPiece");
    }
    if((errno = pthread_create(&scoreHandle, NULL, threadScore, NULL)) != 0) {
       perror("Erreur de lancement du threadScore");
    }
    if((errno = pthread_create(&eventHandle, NULL, threadEvent, NULL)) != 0) {
        perror("Erreur de lancement du threadEvent");
    }
    // Attente de la fin du threadEvent.
    // Le threadEvent se termine quand le joueur clique sur la croix.
    pthread_join(eventHandle, NULL);

    // Fermeture de la grille de jeu (SDL)
    printf("(THREAD MAIN) Fermeture de la grille...");
    FermerGrilleSDL();
    printf("OK\n");

    exit(0);
}

/**
 * Set le message en cours de défilement
 */
void setMessage(const char *text) {
    pthread_mutex_lock(&mutexMessage);
    indiceCourant = 0;
    if(message != NULL) {
        free(message);
    }
    tailleMessage = strlen(text);
    message = (char*) malloc(sizeof(char) * tailleMessage + 1);
    strcpy(message, text);
    pthread_mutex_unlock(&mutexMessage);
}

void* threadDefileMessage(void*) {
    int i;
    struct timespec t;
    printf("(THREAD MESSAGE) Lancement du thread message\n");

    setMessage("Bienvenue dans Tetris Zero Gravity");
    // Message de taille 8 de (11, 10) à (19, 10)

    t.tv_sec = 0;
    t.tv_nsec = 400000000;
    for(;;) {
        pthread_mutex_lock(&mutexMessage);
        for(i = 11; i < 19; ++i) {
            DessineLettre(10, i, getCharFromMessage(i - 11 + indiceCourant));
        }
        ++indiceCourant;
        if(indiceCourant > tailleMessage) {
            indiceCourant = 0;
        }
        pthread_mutex_unlock(&mutexMessage);
        nanosleep(&t, NULL);
    }
}

/**
 * Choisis une pièce aléatoire à insérer
 */
void* threadPiece(void*) {
    printf("(THREAD PIECE) Lancement du thread Piece\n");
    PIECE tmp;
    int shouldNewPiece = 1;
    int i, minX = NB_COLONNES, minY = NB_LIGNES;
    for(;;) {
        pthread_mutex_lock(&mutexPiecesEnCours);
        if(shouldNewPiece) {
            for (i = 0; i < pieceEnCours.nbCases; ++i) {
                EffaceCarre(pieceEnCours.cases[i].ligne + 3, pieceEnCours.cases[i].colonne + 15);
            }
            pieceEnCours = pieces[random(0, 7)];
            for(i = 0; i < random(0, 4); ++i) {
                rotationPiece(&pieceEnCours);
            }
            for (i = 0; i < pieceEnCours.nbCases; ++i) {
                DessineSprite(pieceEnCours.cases[i].ligne + 3,
                    pieceEnCours.cases[i].colonne + 15, pieceEnCours.professeur);
            }
        }
        // Attente de l'insertion de quatre cases
        while(nbCasesInserees < pieceEnCours.nbCases) {
            pthread_cond_wait(&condNbPiecesEnCours, &mutexPiecesEnCours);
        }

        memcpy(tmp.cases, casesInserees, sizeof(casesInserees));
        tmp.nbCases = nbCasesInserees;

        // On ramène les coords en 0,0
        minX = NB_COLONNES;
        minY = NB_LIGNES;
        for(i = 0; i < nbCasesInserees; ++i) {
            if(casesInserees[i].ligne < minY) {
                minY = casesInserees[i].ligne;
            }
            if(casesInserees[i].colonne < minX) {
                minX = casesInserees[i].colonne;
            }
        }
        for(i = 0; i < nbCasesInserees; ++i) {
            tmp.cases[i].ligne -= minY;
            tmp.cases[i].colonne -= minX;
        }
        qsort(tmp.cases, tmp.nbCases, sizeof(CASE), compareCase);

        if(comparaisonPiece(pieceEnCours.cases, tmp.cases, nbCasesInserees)) {
            setPiece(casesInserees, BRIQUE, nbCasesInserees);
            shouldNewPiece = 1;

            // Incrémentation du score
            pthread_mutex_lock(&mutexScore);
            ++score;
            majScore = 1;
            i=0;
            while(i<4){
                pthread_kill(tabThreadCaseHandle[casesInserees[i].ligne][casesInserees[i].colonne],SIGUSR1);
            }
            pthread_cond_signal(&condScore);
            pthread_mutex_unlock(&mutexScore);

            printf("(THREAD PIECE) Yipee!\n");
        } else {
            setPiece(casesInserees, VIDE, nbCasesInserees);
            shouldNewPiece = 0;
            printf("(THREAD PIECE) Boohh!\n");
        }

        nbCasesInserees = 0;
        pthread_mutex_unlock(&mutexPiecesEnCours);
    }
}

/**
 * Gestion des inputs souris
 */
void* threadEvent(void*){
    printf("(THREAD EVENT) Lancement du thread threadEvent\n");
    EVENT_GRILLE_SDL event;

    for(;;) {
        event = ReadEvent();
        switch(event.type) {
            case CROIX:
                return NULL;

            case CLIC_GAUCHE:
                pthread_mutex_lock(&mutexTab);
                if(event.colonne < 10 && tab[event.colonne][event.ligne] == 0) {
                    printf("(THREAD EVENT) Clic gauche\n");
                    pthread_mutex_lock(&mutexPiecesEnCours);
                    casesInserees[nbCasesInserees].ligne = event.ligne;
                    casesInserees[nbCasesInserees].colonne = event.colonne;
                    ++nbCasesInserees;
                    tab[event.colonne][event.ligne] = 1;

                    DessineSprite(event.ligne, event.colonne, pieceEnCours.professeur);
                    pthread_mutex_unlock(&mutexPiecesEnCours);
                    pthread_cond_signal(&condNbPiecesEnCours);
                }
                pthread_mutex_unlock(&mutexTab);
                break;

            case CLIC_DROIT:
                printf("(THREAD EVENT) Clic droit\n");
                pthread_mutex_lock(&mutexPiecesEnCours);
                if(nbCasesInserees == 0) {
                    pthread_mutex_unlock(&mutexPiecesEnCours);
                    break;
                }
                --nbCasesInserees;
                pthread_mutex_lock(&mutexTab);
                tab[casesInserees[nbCasesInserees].colonne][casesInserees[nbCasesInserees].ligne] = 0;
                pthread_mutex_unlock(&mutexTab);
                EffaceCarre(casesInserees[nbCasesInserees].ligne, casesInserees[nbCasesInserees].colonne);
                pthread_mutex_unlock(&mutexPiecesEnCours);
                pthread_cond_signal(&condNbPiecesEnCours);

                break;
        }
    }
}

void *threadScore(void *a) {
    printf("(THREAD SCORE) Lancement du thread Score\n");
    char buff[5];
    pthread_mutex_lock(&mutexScore);
    while(!majScore) {
        printf("(THREAD SCORE) MAJ du score : '%4d'\n", score);
        pthread_cond_wait(&condScore, &mutexScore);
        sprintf(buff, "%4d", score);
        DessineSprite(1, 15, CHIFFRE_0 + (buff[0] == ' ' ? 0 : buff[0] - '0'));
        DessineSprite(1, 16, CHIFFRE_0 + (buff[1] == ' ' ? 0 : buff[1] - '0'));
        DessineSprite(1, 17, CHIFFRE_0 + (buff[2] == ' ' ? 0 : buff[2] - '0'));
        DessineSprite(1, 18, CHIFFRE_0 + (buff[3] == ' ' ? 0 : buff[3] - '0'));
        majScore = 0;
    }
    pthread_mutex_unlock(&mutexScore);
}

void *threadCase(void*p){

    pthread_key_create(&keyCase,suppressionCase);
    CASE* maCase = (CASE*)malloc(sizeof(CASE));
    pthread_setspecific(keyCase,&maCase);
    *maCase = *(CASE*)p;
    pthread_mutex_unlock(&mutexMaCase);

    printf("(THREAD CASE) Lancement du thread case L:%d C:%d\n",maCase->ligne,maCase->colonne);

    for (;;)
    {
        pause();
    }
}


/**
 * Retourne la partie du message défilant
 * correspondant au défilement courant
 */
char getCharFromMessage(int index) {
    return message[index % (tailleMessage+1)];
}

/**
 * Tourne une pièce
 */
void rotationPiece(PIECE *piece) {
    if(piece->professeur == WAGNER) {
        // Inutile de faire la rotation d'un carré
        return;
    }

    // Multiplication par une matrice
    // 0 -1
    // 1  0

    // Correspondent à la translation à effectuer après la rotation
    int smallestLigne = 0, smallestColonne = 0;
    CASE tmpCase;
    int i;

    for(i = 0; i < piece->nbCases; ++i) {
        // Rotation de la case i
        tmpCase.ligne = -piece->cases[i].colonne;
        tmpCase.colonne = piece->cases[i].ligne;

        // Insertion
        piece->cases[i] = tmpCase;

        // Récupération de la translation à effectuer ensuite
        if(tmpCase.ligne < smallestLigne) {
            smallestLigne = tmpCase.ligne;
        }
        if(tmpCase.colonne < smallestColonne) {
            smallestColonne = tmpCase.colonne;
        }
    }

    // Translation pour garder des coordonnées positives
    for(i = 0; i < piece->nbCases; ++i) {
        piece->cases[i].colonne -= smallestColonne;
        piece->cases[i].ligne -= smallestLigne;
    }

    qsort(piece->cases, piece->nbCases, sizeof(CASE), compareCase);
}

/**
 * Compare deux cases pour voir si l'une
 * d'elle devrait être placée devant dans la liste de case de la pièce
 */
int compareCase(const void *a, const void *b) {
    CASE *c1 = (CASE*) a;
    CASE *c2 = (CASE*) b;

    if(c1->ligne < c2->ligne) {
        return -1;
    }
    if(c1->ligne > c2->ligne) {
        return 1;
    }
    if(c1->colonne < c2->colonne) {
        return -1;
    }
    if(c1->colonne > c2->colonne) {
        return 1;
    }
    return 0;
}

/**
 * Retourne un nombre aléatoire entre bornes
 */
int random(int min, int max) {
    return min + rand() % (max - min);
}

/**
 * Compare les coordonées de deux pièces via les coords de leurs cases
 */
int comparaisonPiece(CASE p1[], CASE p2[], int nbCases) {
    for(int i = 0; i < nbCases; ++i) {
        if(p1[i].ligne != p2[i].ligne || p1[i].colonne != p2[i].colonne) {
            return 0;
        }
    }
    return 1;
}

/**
 * Modifie toutes les cases d'une pièce avec le type souhaité
 */
void setPiece(CASE cases[], int type, int nbCases) {
    for(int i = 0; i < nbCases; ++i) {
        if (type == VIDE) {
            EffaceCarre(cases[i].ligne, cases[i].colonne);
            pthread_mutex_lock(&mutexTab);
            tab[cases[i].colonne][cases[i].ligne] = 0;
            pthread_mutex_unlock(&mutexTab);
        } else {
            DessineSprite(cases[i].ligne, cases[i].colonne, type);
        }
    }
}

void suppressionCase(void*){
    free (pthread_getspecific(keyCase));
}

void handlerSIGUSR1(int sig){
    CASE* maCase=(CASE*)pthread_getspecific(keyCase);
    int i;
    printf("(HANDLER SIGUSR1) START\n");


    for (i = 0; i < 14; ++i){
        pthread_mutex_lock(&mutexTab);
        if (tab[i][maCase->colonne]==0){
            break;
        }

        pthread_mutex_unlock(&mutexTab);
    }
    if (i==13 && tab[13][maCase->colonne]==0){
        pthread_mutex_lock(&mutexAnalyse);

        i=0;
        while(i<4){
            if (colonnesCompletes[i]==maCase->colonne){
                break;
            }
            ++i;
        }
        if (i==4){
            printf("(HANDLER SIGUSR1) COLONNE FILLED\n");
            colonnesCompletes[nbColonnesCompletes]=maCase->colonne;
            ++nbColonnesCompletes;
        }
        pthread_mutex_unlock(&mutexAnalyse);
    }

    for (i = 0; i < 10; ++i){
        pthread_mutex_lock(&mutexTab);
        if (tab[maCase->ligne][i]==0){
            break;
        }
        pthread_mutex_unlock(&mutexTab);
    }
    if (i==13 && tab[maCase->ligne][i]==0)
    {
        pthread_mutex_lock(&mutexAnalyse);

        i=0;
        while(i<4){
            if (lignesCompletes[i]==maCase->ligne){
                break;
            }
            ++i;
        }
        if (i==4){
            lignesCompletes[nbLignesCompletes]=maCase->ligne;
            ++nbLignesCompletes;
        }
        pthread_mutex_unlock(&mutexAnalyse);
    }
}
