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

void setMessage(const char *texte);
void* threadDefileMessage(void*);
void* threadEvent(void*);
char getCharFromMessage(int index);

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

PIECE pieces[7] = { 0, 0, 0, 1, 1, 0, 1, 1, 4, WAGNER,       // carre
                    0, 0, 1, 0, 2, 0, 2, 1, 4, MERCENIER,    // L
                    0, 1, 1, 1, 2, 0, 2, 1, 4, VILVENS,      // J
                    0, 0, 0, 1, 1, 1, 1, 2, 4, DEFOOZ,       // Z
                    0, 1, 0, 2, 1, 0, 1, 1, 4, GERARD,       // S
                    0, 0, 0, 1, 0, 2, 1, 1, 4, CHARLET,      // T
                    0, 0, 0, 1, 0, 2, 0, 3, 4, MADANI };     // I

char* message = NULL; // pointeur vers le message à faire défiler
int tailleMessage; // longueur du message
int indiceCourant; // indice du premier caractère à afficher dans la zone graphique
PIECE pieceEnCours; // Pièce en cours de placement
CASE casesInserees[NB_CASES]; // cases insérées par le joueur
int nbCasesInserees;  // nombre de cases actuellement insérées par le joueur.

pthread_mutex_t mutexMessage; // Mutex pour message, tailleMessage et indiceCourant


///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    pthread_mutex_init(&mutexMessage, NULL);

    char buffer[80];
    char ok = 0;
    pthread_t defileMessageHandle;
    pthread_t pieceHandle;
    pthread_t eventHandle;

    srand((unsigned)time(NULL));

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

    if((errno = pthread_create(&defileMessageHandle, NULL, threadDefileMessage, NULL)) != 0) {
        perror("Erreur de lancement de thread");
    }
    pthread_detach(defileMessageHandle);

    //if((errno = pthread_create(&pieceHandle, NULL, rotationPiece, NULL)) != 0) {
    //    perror("Erreur de lancement de thread");
    //}
    //pthread_detach(pieceHandle);

	if((errno = pthread_create(&eventHandle, NULL, threadEvent, NULL)) != 0) {
        perror("Erreur de lancement de thread");
    }

    pthread_join(eventHandle, NULL); // Attente de la fin du threadEvent. Le threadEvent se termine quand le joueur clique sur la croix.
    // Fermeture de la grille de jeu (SDL)
    printf("(THREAD MAIN) Fermeture de la grille..."); fflush(stdout);
    FermerGrilleSDL();
    printf("OK\n"); fflush(stdout);

    exit(0);
}

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
        for (i = 11; i < 19; ++i) {
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

void* threadEvent(void*){

    printf("(THREAD MESSAGE) Lancement du thread threadEvent\n");
    EVENT_GRILLE_SDL event;
    int ok = 0;

    while(!ok) {
        event = ReadEvent();
        if (event.type == CROIX) {
            ok = 1;
        }
        if (event.type == CLIC_GAUCHE) {
            if(event.colonne < 10) {
                DessineSprite(event.ligne, event.colonne, WAGNER);
            }
        }
    }
}


char getCharFromMessage(int index) {
    return message[index % (tailleMessage+1)];
}

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
    int i, j, indiceSmallest;

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
    for (i = 0; i < piece->nbCases; ++i) {
        piece->cases[i].colonne += -smallestColonne;
        piece->cases[i].ligne += -smallestLigne;
    }

    tmpCase = piece->cases[0];
    for (i = 0; i < piece->nbCases; ++i) {
        for (j = i; j < piece->nbCases; ++j) {
            if(piece->cases[j].ligne < tmpCase.ligne ||
                    piece->cases[j].colonne < tmpCase.colonne
                    && piece->cases[j].ligne >= tmpCase.ligne) {
                tmpCase = piece->cases[j];
                indiceSmallest = j;
            }
        }
        piece->cases[indiceSmallest] = piece->cases[i];
        piece->cases[i] = tmpCase;
    }
}
