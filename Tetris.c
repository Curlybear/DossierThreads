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
int   lignesCompletes[4];
int   nbLignesCompletes;
int   colonnesCompletes[4];
int   nbColonnesCompletes;
int   nbAnalyses = 0;
int   traitementEnCours = 0;
key_t cle;                     // Clé passée en paramètre (utile pour la connexion au serveur)
char *pseudo;                  // Pseudo du joueur en cours

// Thread Handle
pthread_t threadCaseHandle[14][10];
pthread_t finPartieHandle;

// Key Specific
pthread_key_t keyCase;

// Mutex's
pthread_mutex_t mutexTab;
pthread_mutex_t mutexMessage; // Mutex pour message, tailleMessage et indiceCourant
pthread_mutex_t mutexPiecesEnCours;
pthread_mutex_t mutexScore;
pthread_mutex_t mutexParamThreadCase;
pthread_mutex_t mutexAnalyse;
pthread_mutex_t mutexTraitement;

// Cond's
pthread_cond_t condNbPiecesEnCours;
pthread_cond_t condScore;
pthread_cond_t condAnalyse;

///////////////////////////////////////////////////////////////////////////////////////////////////
void  setMessage(const char*);
char  getCharFromMessage(int);
void  rotationPiece(PIECE*);
int   compareCase(const void*, const void*);
int   random(int, int);
int   comparaisonPiece(CASE[], CASE[], int);
void  setPiece(CASE[], int, int);
void  gravityVectorSorting(int[], int, int);
void  setTraitementEnCours(int);

void  suppressionCase(void*);
void  freeMessage(void*);
void  decoServeur(void*);


// THREADS
void *threadDefileMessage(void*);
void *threadPiece(void*);
void *threadEvent(void*);
void *threadScore(void*);
void *threadCase(void*);
void *threadGravite(void*);
void *threadFinPartie(void*);
void *threadJoueursConnectes(void *);

// Sig Handlers
void handlerSIGUSR1(int sig);
void handlerSIGUSR2(int sig);
void handlerSIGHUP(int sig);

// DEBUG
void displayTab();

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
    int i, j;
    char buffer[80];
    char ok = 0;
    CASE tmpCase;
    EVENT_GRILLE_SDL event;

    // Initialisation de pieceEnCours dans le cas où
    // le thread gravité se lance avant le thread pièce.
    pieceEnCours=pieces[0];

    pthread_key_create(&keyCase, suppressionCase);

    pthread_t defileMessageHandle;
    pthread_t pieceHandle;
    pthread_t eventHandle;
    pthread_t scoreHandle;
    pthread_t graviteHandle;
    pthread_t joueursConnectesHandle;

    srand((unsigned)time(NULL));

    pthread_mutex_init(&mutexTab, NULL);
    pthread_mutex_init(&mutexMessage, NULL);
    pthread_mutex_init(&mutexPiecesEnCours, NULL);
    pthread_mutex_init(&mutexScore, NULL);
    pthread_mutex_init(&mutexParamThreadCase, NULL);
    pthread_mutex_init(&mutexAnalyse, NULL);
    pthread_mutex_init(&mutexTraitement, NULL);

    pthread_cond_init(&condNbPiecesEnCours, NULL);
    pthread_cond_init(&condScore, NULL);
    pthread_cond_init(&condAnalyse, NULL);

    // Ouverture de la grille de jeu (SDL)
    printf("(THREAD MAIN) Ouverture de la grille de jeu\n");
    fflush(stdout);
    sprintf(buffer, "!!! TETRIS ZERO GRAVITY !!!");
    if(OuvrirGrilleSDL(NB_LIGNES, NB_COLONNES, 40, buffer) < 0) {
        printf("Erreur de OuvrirGrilleSDL\n");
        fflush(stdout);
        exit(1);
    }

    // Chargement des sprites et de l'image de fond
    ChargementImages();
    DessineSprite(12, 11, VOYANT_VERT);

    // Initialisation des trucs de signaux
    struct sigaction sigAct;
    sigset_t mask;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = 0;
    sigemptyset(&mask);

    // On ne se connecte au serveur que si on a les arguments
    if(argc == 3) {
        ++argv;
        cle = atoi(*argv++);
        pseudo = (char*) malloc(strlen(*argv) + 1);
        strcpy(pseudo, *argv);

        // Armement de SIGHUP pour threadJoueursConnectes
        sigAct.sa_handler = handlerSIGHUP;
        sigaction(SIGHUP, &sigAct, NULL);
        if(pthread_create(&joueursConnectesHandle, NULL, threadJoueursConnectes, NULL) != 0) {
            fprintf(stderr, "Erreur de lancement de threadJoueursConnectes\n");
        }
    }

    // Masquage pour les threads suivants
    sigaddset(&mask, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    // Armement de SIGUSR1
    sigAct.sa_handler = handlerSIGUSR1;
    sigaction(SIGUSR1, &sigAct, NULL);

    pthread_mutex_lock(&mutexParamThreadCase);
    for(i = 0; i < 14; ++i) {
        for(j = 0; j < 10; ++j) {
            tmpCase.ligne = i;
            tmpCase.colonne = j;

            if(pthread_create(&threadCaseHandle[i][j], NULL, threadCase, &tmpCase) != 0) {
                fprintf(stderr, "Erreur de lancement du threadCase[%d][%d]", i, j);
            }
            pthread_mutex_lock(&mutexParamThreadCase);
        }
    }
    pthread_mutex_unlock(&mutexParamThreadCase);

    // Masquage pour les threads suivants
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    sigAct.sa_handler=handlerSIGUSR2;
    sigAct.sa_flags = 0;
    sigaction(SIGUSR2, &sigAct, NULL);

    if(pthread_create(&finPartieHandle, NULL, threadFinPartie, NULL) != 0) {
        fprintf(stderr, "Erreur de lancement du threadFinPartie");
    }

    // Masquage pour les threads suivants
    sigaddset(&mask, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    if(pthread_create(&defileMessageHandle, NULL, threadDefileMessage, NULL) != 0) {
        perror("Erreur de lancement du threadDefileMessage");
    }
    pthread_detach(defileMessageHandle);
    if(pthread_create(&pieceHandle, NULL, threadPiece, NULL) != 0) {
       perror("Erreur de lancement du threadPiece");
    }
    if(pthread_create(&scoreHandle, NULL, threadScore, NULL) != 0) {
       perror("Erreur de lancement du threadScore");
    }
    if(pthread_create(&graviteHandle, NULL, threadGravite, NULL) != 0) {
       perror("Erreur de lancement du threadGravite");
    }
    if(pthread_create(&eventHandle, NULL, threadEvent, NULL) != 0) {
        perror("Erreur de lancement du threadEvent");
    }
    // Attente de la fin du threadEvent.
    // Le threadEvent se termine quand le joueur clique sur la croix.
    pthread_join(finPartieHandle, NULL);

    pthread_cancel(eventHandle);
    for(i = 0; i < 14; ++i) {
        for(j = 0; j < 10; ++j) {
            pthread_cancel(threadCaseHandle[i][j]);
        }
    }

    for(;;) {
        event = ReadEvent();
        if (event.type == CROIX) {
            break;
        }
    }

    pthread_cancel(defileMessageHandle);
    free(pseudo);

    // Fermeture de la grille de jeu (SDL)
    printf("(THREAD MAIN) Fermeture de la grille...");
    FermerGrilleSDL();
    printf("OK\n");
    pthread_cancel(joueursConnectesHandle);
    printf("DEBUG\n");

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
    pthread_cleanup_push(freeMessage, NULL);

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
    pthread_cleanup_pop(1);
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
            for(i = 0; i < pieceEnCours.nbCases; ++i) {
                EffaceCarre(pieceEnCours.cases[i].ligne + 3, pieceEnCours.cases[i].colonne + 15);
            }
            // TODO Remettre ça comme il faut!
            // pieceEnCours = pieces[random(0, 7)];
            pieceEnCours = pieces[0]; // DEBUG!!!!!!!!
            // for(i = 0; i < random(0, 4); ++i) {
            //     rotationPiece(&pieceEnCours);
            // }
            for(i = 0; i < pieceEnCours.nbCases; ++i) {
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
            pthread_cond_signal(&condScore);
            pthread_mutex_unlock(&mutexScore);

            // Analyse des lignes et des colonnes
            pthread_mutex_lock(&mutexAnalyse);
            nbColonnesCompletes = 0;
            nbLignesCompletes = 0;
            /* Inutile, reset par le thread gravite...
            nbAnalyses = 0; */
            pthread_mutex_unlock(&mutexAnalyse);
            i = 0;
            while(i < nbCasesInserees) {
                pthread_kill(threadCaseHandle[casesInserees[i].ligne][casesInserees[i].colonne], SIGUSR1);
                ++i;
            }

            printf("(THREAD PIECE) Yipee!\n");
        } else {
            setTraitementEnCours(0);
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
void* threadEvent(void*) {
    printf("(THREAD EVENT) Lancement du thread threadEvent\n");
    EVENT_GRILLE_SDL event;

    for(;;) {
        event = ReadEvent();
        switch(event.type) {
            case CROIX:
                pthread_cancel(finPartieHandle); // TODO This seems to be buggy...

            case CLIC_GAUCHE:
                pthread_mutex_lock(&mutexTab);
                if(event.colonne < 10 && tab[event.ligne][event.colonne] == 0 && !traitementEnCours) {
                    // printf("(THREAD EVENT) Clic gauche\n");
                    pthread_mutex_lock(&mutexPiecesEnCours);
                    casesInserees[nbCasesInserees].ligne = event.ligne;
                    casesInserees[nbCasesInserees].colonne = event.colonne;
                    ++nbCasesInserees;
                    tab[event.ligne][event.colonne] = 1;

                    DessineSprite(event.ligne, event.colonne, pieceEnCours.professeur);
                    if(nbCasesInserees == pieceEnCours.nbCases) {
                        setTraitementEnCours(1);
                    }
                    pthread_cond_signal(&condNbPiecesEnCours);
                    pthread_mutex_unlock(&mutexPiecesEnCours);
                } else {
                    DessineSprite(12, 11, VOYANT_ROUGE);
                    struct timespec t;
                    t.tv_sec = 0;
                    t.tv_nsec = 250000000,
                    nanosleep(&t, NULL);
                    DessineSprite(12, 11, traitementEnCours ? VOYANT_BLEU : VOYANT_VERT);
                }
                pthread_mutex_unlock(&mutexTab);
                break;

            case CLIC_DROIT:
                // printf("(THREAD EVENT) Clic droit\n");
                pthread_mutex_lock(&mutexPiecesEnCours);
                if(nbCasesInserees != 0) {
                    --nbCasesInserees;
                    pthread_mutex_lock(&mutexTab);
                    tab[casesInserees[nbCasesInserees].ligne][casesInserees[nbCasesInserees].colonne] = 0;
                    pthread_mutex_unlock(&mutexTab);
                    EffaceCarre(casesInserees[nbCasesInserees].ligne, casesInserees[nbCasesInserees].colonne);
                    pthread_cond_signal(&condNbPiecesEnCours);
                }
                pthread_mutex_unlock(&mutexPiecesEnCours);
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

/**
 * Ce thread est crée autant de fois qu'il y a de case,
 * Il est réveillé à la réception du SIGUSR1, càd, si
 * une pièce a correctement été posée sur la case correspondante...
 *
 * @param CASE *p Les coordonées de la case correspondant au thread
 */
void *threadCase(void *p) {
    CASE *tmpCase = (CASE*) malloc(sizeof(CASE));
    if(!tmpCase) {
        fprintf(stderr, "(THREAD CASE) malloc fail...\n");
    }
    if(pthread_setspecific(keyCase, tmpCase)) {
        fprintf(stderr, "(THREAD CASE) pthread_setspecific fail...\n");
    }
    *tmpCase = *(CASE*) p;
    pthread_mutex_unlock(&mutexParamThreadCase);

    for(;;) {
        pause();
    }
}

/**
 * Applique la gravité si le threadCase à trouvé des lignes/colonnes
 * complètes à supprimer
 */
void *threadGravite(void *p) {
    int i, j, k;
    struct timespec t1;
    struct timespec t2;
    t1.tv_sec = 1;
    t1.tv_nsec = 500000000;
    t2.tv_sec = 0;
    t2.tv_nsec = 500000000;

    for(;;) {
        // Attente de l'analyse
        pthread_mutex_lock(&mutexAnalyse);
        while(nbAnalyses < pieceEnCours.nbCases) {
            pthread_cond_wait(&condAnalyse, &mutexAnalyse);
        }
        // Reset du nombre d'analyse
        nbAnalyses = 0;
        pthread_mutex_unlock(&mutexAnalyse);

        // On passe son tour si pas de ligne / colonne complète
        if(nbColonnesCompletes <= 0 && nbLignesCompletes <= 0) {
            pthread_kill(finPartieHandle, SIGUSR2);
            continue;
        }

        printf("(THREAD GRAVITE) Starting...\n");
        // Attente de 1.5 secondes + 0.5 un peu plus loin...
        nanosleep(&t1, NULL);

        // TODO Trouver un truc générique histoire de pas se taper le même code 4 fois...

        // Tri des lignes et des colonnes complètes
        gravityVectorSorting(lignesCompletes, nbLignesCompletes, 7);
        gravityVectorSorting(colonnesCompletes, nbColonnesCompletes, 5);

        // Début de la gravité des lignes
        for(i = 0; i < nbLignesCompletes; ++i) {
            printf("Suppression de la ligne %d\n", lignesCompletes[i]);
            nanosleep(&t2, NULL);
            if(lignesCompletes[i] < 7) {
                // Mouvement vers le bas
                for(j = lignesCompletes[i]; j != 0; --j) {
                    for(k = 0; k < 10; ++k) {
                        pthread_mutex_lock(&mutexTab);
                        tab[j][k] = tab[j-1][k];
                        if(tab[j][k]) {
                            DessineSprite(j, k, BRIQUE);
                        } else {
                            EffaceCarre(j, k);
                        }
                        pthread_mutex_unlock(&mutexTab);
                    }
                }
                // Suppression de la première ligne
                for(j = 0; j < 10; ++j) {
                    pthread_mutex_lock(&mutexTab);
                    tab[0][j] = 0;
                    EffaceCarre(0, j);
                    pthread_mutex_unlock(&mutexTab);
                }
            } else {
                // Mouvement vers le haut
                for(j = lignesCompletes[i]; j < 13; ++j) {
                    for(k = 0; k < 10; ++k) {
                        pthread_mutex_lock(&mutexTab);
                        tab[j][k] = tab[j+1][k];
                        if(tab[j][k]) {
                            DessineSprite(j, k, BRIQUE);
                        } else {
                            EffaceCarre(j, k);
                        }
                        pthread_mutex_unlock(&mutexTab);
                    }
                }
                // Suppression de la dernière ligne
                for(j = 0; j < 10; ++j) {
                    pthread_mutex_lock(&mutexTab);
                    tab[13][j] = 0;
                    EffaceCarre(13, j);
                    pthread_mutex_unlock(&mutexTab);
                }
            }
        }

        // Début de la gravité des colonnes
        for(i = 0; i < nbColonnesCompletes; ++i) {
            printf("Suppression de la colonne %d\n", colonnesCompletes[i]);
            nanosleep(&t2, NULL);
            if(colonnesCompletes[i] < 5) {
                // Mouvement vers le droite
                for(j = 0; j < NB_LIGNES; ++j) {
                    for(k = colonnesCompletes[i]; k != 0; --k) {
                        pthread_mutex_lock(&mutexTab);
                        tab[j][k] = tab[j][k-1];
                        if(tab[j][k]) {
                            DessineSprite(j, k, BRIQUE);
                        } else {
                            EffaceCarre(j, k);
                        }
                        pthread_mutex_unlock(&mutexTab);
                    }
                }
                // Suppression de la première colonne
                for(j = 0; j < NB_LIGNES; ++j) {
                    pthread_mutex_lock(&mutexTab);
                    tab[j][0] = 0;
                    EffaceCarre(j, 0);
                    pthread_mutex_unlock(&mutexTab);
                }
            } else {
                // Mouvement vers la gauche
                for(j = 0; j < NB_LIGNES; ++j) {
                    for(k = colonnesCompletes[i]; k < 10; ++k) {
                        pthread_mutex_lock(&mutexTab);
                        tab[j][k] = tab[j][k+1];
                        if(tab[j][k]) {
                            DessineSprite(j, k, BRIQUE);
                        } else {
                            EffaceCarre(j, k);
                        }
                        pthread_mutex_unlock(&mutexTab);
                    }
                }
                // Suppression de la dernière colonne
                for(j = 0; j < NB_LIGNES; ++j) {
                    pthread_mutex_lock(&mutexTab);
                    tab[j][9] = 0;
                    EffaceCarre(j, 9);
                    pthread_mutex_unlock(&mutexTab);
                }
            }
        }

        // Ajout du score
        pthread_mutex_lock(&mutexScore);
        score += 5 * (nbColonnesCompletes + nbLignesCompletes);
        pthread_cond_signal(&condScore);
        pthread_mutex_unlock(&mutexScore);

        pthread_kill(finPartieHandle, SIGUSR2);

    }
}

void *threadFinPartie(void *) {
    for(;;) {
        pause();
        setTraitementEnCours(0);
    }
}

void *threadJoueursConnectes(void *p) {
    printf("(THREAD JOUEURS CONNECTES) starting with '%d' : '%s'\n", cle, pseudo);
    pthread_cleanup_push(decoServeur, NULL);

    if(ConnectionServeur(cle, pseudo) != 0) {
        fprintf(stderr, "(THREAD JOUEURS CONNECTES) Erreur de connexion au serveur\n");
        pthread_exit(NULL);
    }

    for(;;) {
        pause();
    }
    pthread_cleanup_pop(1);
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
        if(type == VIDE) {
            EffaceCarre(cases[i].ligne, cases[i].colonne);
            pthread_mutex_lock(&mutexTab);
            tab[cases[i].ligne][cases[i].colonne] = 0;
            pthread_mutex_unlock(&mutexTab);
        } else {
            DessineSprite(cases[i].ligne, cases[i].colonne, type);
        }
    }
}

void suppressionCase(void *p) {
    free(pthread_getspecific(keyCase));
}

void handlerSIGUSR1(int sig) {
    // printf("(HANDLER SIGUSR1) start\n");
    CASE *tmpCase = (CASE*) pthread_getspecific(keyCase);
    if(!tmpCase) {
        fprintf(stderr, "pthread_getspecific fail...\n");
        exit(1);
    }
    int i;

    // Check si la colonne est complète
    for(i = 0; i < 14; ++i) {
        pthread_mutex_lock(&mutexTab);
        if(tab[i][tmpCase->colonne] == 0) {
            pthread_mutex_unlock(&mutexTab);
            break;
        }
        pthread_mutex_unlock(&mutexTab);
    }

    if(i == 14) {
        pthread_mutex_lock(&mutexAnalyse);
        i = 0;
        while(i < nbColonnesCompletes) {
            if(colonnesCompletes[i] == tmpCase->colonne) {
                break;
            }
            ++i;
        }
        if(i == nbColonnesCompletes) {
            printf("(HANDLER SIGUSR1) colonne %d complete\n", tmpCase->colonne);
            colonnesCompletes[nbColonnesCompletes] = tmpCase->colonne;
            ++nbColonnesCompletes;

            for(i = 0; i < 14; ++i) {
                DessineSprite(i,tmpCase->colonne,FUSION);
            }
        }
        pthread_mutex_unlock(&mutexAnalyse);
    }

    // Check si la ligne est complète
    for(i = 0; i < 10; ++i) {
        pthread_mutex_lock(&mutexTab);
        if(tab[tmpCase->ligne][i] == 0) {
            pthread_mutex_unlock(&mutexTab);
            break;
        }
        pthread_mutex_unlock(&mutexTab);
    }

    if(i == 10) {
        pthread_mutex_lock(&mutexAnalyse);
        i = 0;
        while(i < nbLignesCompletes) {
            if(lignesCompletes[i] == tmpCase->ligne) {
                break;
            }
            ++i;
        }
        if(i == nbLignesCompletes) {
            printf("(HANDLER SIGUSR1) ligne %d complete\n", tmpCase->ligne);
            lignesCompletes[nbLignesCompletes] = tmpCase->ligne;
            ++nbLignesCompletes;
            for(i = 0; i < 10; ++i) {
                DessineSprite(tmpCase->ligne,i,FUSION);
            }
        }
        pthread_mutex_unlock(&mutexAnalyse);
    }
    pthread_mutex_lock(&mutexAnalyse);
    ++nbAnalyses;
    pthread_cond_signal(&condAnalyse);
    pthread_mutex_unlock(&mutexAnalyse);
}

void handlerSIGUSR2(int sig){
    int i,j,k;

    for(i = 0; i < NB_LIGNES; ++i) {
        for(j = 0; j < 10; ++j) {
            pthread_mutex_lock(&mutexPiecesEnCours);
            pthread_mutex_lock(&mutexTab);

            for(k = 0; k < 4; ++k) {
                if(i+pieceEnCours.cases[k].ligne < NB_LIGNES && j+pieceEnCours.cases[k].colonne < 10) {
                    if(tab[i+pieceEnCours.cases[k].ligne][j+pieceEnCours.cases[k].colonne] == 1) {
                        break;
                    }
                } else {
                    break;
                }
            }
            pthread_mutex_unlock(&mutexTab);
            pthread_mutex_unlock(&mutexPiecesEnCours);
            if(k == 4) {
                pthread_mutex_lock(&mutexTraitement);
                pthread_mutex_unlock(&mutexTraitement);
                return;
            }
        }
    }
    printf("(HANDLER SIGUSR2) Plus de place");

    pthread_exit(NULL);
}

/**
 * Fonction lancée lorsque le serveur envoie un SIGHUP
 * (càd, lorsqu'un joueur se (dé)connecte.)
 */
void handlerSIGHUP(int sig) {
    int nb = GetNbJoueursConnectes(cle);
    printf("(THREAD JOUEURS CONNECTES) Mise à jours du nombre de joueurs connectes : %d\n", nb);
    char buff[3];
    if(nb > 0) {
        sprintf(buff, "%2d", nb);
        DessineSprite(12, 17, CHIFFRE_0 + (buff[0] == ' ' ? 0 : buff[0] - '0'));
        DessineSprite(12, 18, CHIFFRE_0 + (buff[1] == ' ' ? 0 : buff[1] - '0'));
    }
}

/**
 * Trie un vecteur par ordre croissant si en dessous du centre
 * et par ordre décroissant sinon (cf. énoncé p9).
 */
void gravityVectorSorting(int vector[], int size, int center) {
    int i, j, k;
    for(i = 0; i < size; ++i) {
        for(j = i; j < size; ++j) {
            if(vector[i] < center) {
                if(vector[j] < vector[i]) {
                    k = vector[j];
                    vector[j] = vector[i];
                    vector[i] = k;
                }
            } else {
                if(vector[j] > vector[i]) {
                    k = vector[j];
                    vector[j] = vector[i];
                    vector[i] = k;
                }
            }
        }
    }
}

/**
 * Défini si il y a un traitement en cours
 */
void setTraitementEnCours(int enCours) {
    pthread_mutex_lock(&mutexTraitement);
    traitementEnCours = enCours;
    pthread_mutex_unlock(&mutexTraitement);
    if(enCours) {
        DessineSprite(12, 11, VOYANT_BLEU);
    } else {
        DessineSprite(12, 11, VOYANT_VERT);
    }
}

void freeMessage(void*) {
    printf("(FreeMessage) Free de message\n");
    free(message);
}

void decoServeur(void*) {
    printf("(decoServeur) deco du serveur\n");
    DeconnectionServeur(cle);
}


///////////////////////////////////////////////////////////////////
// DEBUG FUNCTIONS
void displayTab() {
    printf("===============================\n");
    for(int i = 0; i < NB_LIGNES; ++i) {
        for(int j = 0; j < 10; ++j) {
            pthread_mutex_lock(&mutexTab);
            printf("%d | ", tab[i][j]);
            pthread_mutex_unlock(&mutexTab);
        }
        printf("\n");
    }
    printf("===============================\n");
}
