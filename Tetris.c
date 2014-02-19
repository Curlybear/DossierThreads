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
void* defileMessage(void*);
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

typedef struct
{
  int ligne;
  int colonne;
} CASE;

typedef struct
{
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

pthread_mutex_t mutexMessage; // Mutex pour message, tailleMessage et indiceCourant


///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    pthread_mutex_init(&mutexMessage, NULL);

    EVENT_GRILLE_SDL event;
    char buffer[80];
    char ok;
    pthread_t threadDefileMessage;

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

    if((errno = pthread_create(&threadDefileMessage, NULL, defileMessage, NULL)) != 0) {
        perror("Erreur de lancement de thread");
    }
    pthread_detach(threadDefileMessage);

    ok = 0;

    while(!ok) {
        event = ReadEvent();
        if (event.type == CROIX)
            ok = 1;
        if (event.type == CLIC_GAUCHE)
            DessineSprite(event.ligne, event.colonne, WAGNER);
    }

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

void* defileMessage(void*) {
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

char getCharFromMessage(int index) {
    return message[index % (tailleMessage+1)];
}
