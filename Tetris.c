#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
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
={ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
  
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

PIECE pieces[7] = { 0,0,0,1,1,0,1,1,4,WAGNER,       // carre
                    0,0,1,0,2,0,2,1,4,MERCENIER,    // L
                    0,1,1,1,2,0,2,1,4,VILVENS,      // J
                    0,0,0,1,1,1,1,2,4,DEFOOZ,       // Z
                    0,1,0,2,1,0,1,1,4,GERARD,       // S
                    0,0,0,1,0,2,1,1,4,CHARLET,      // T
                    0,0,0,1,0,2,0,3,4,MADANI };     // I


///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char* argv[])
{
  EVENT_GRILLE_SDL event;
  char buffer[80];
  char ok;
 
  srand((unsigned)time(NULL));

  // Ouverture de la grille de jeu (SDL)
  printf("(THREAD MAIN) Ouverture de la grille de jeu\n");
  fflush(stdout);
  sprintf(buffer,"!!! TETRIS ZERO GRAVITY !!!");
  if (OuvrirGrilleSDL(NB_LIGNES,NB_COLONNES,40,buffer) < 0)
  {
    printf("Erreur de OuvrirGrilleSDL\n");
    fflush(stdout);
    exit(1);
  }

  // Chargement des sprites et de l'image de fond
  ChargementImages();
  DessineSprite(12,11,VOYANT_VERT);

  // Exemple d'utilisation de GrilleSDL et Ressources --> code a supprimer
  DessineLettre(10,12,'e');
  DessineChiffre(10,14,4);
  ok = 0;
  while(!ok)
  {
    event = ReadEvent();
    if (event.type == CROIX) ok = 1;
    if (event.type == CLIC_GAUCHE) DessineSprite(event.ligne,event.colonne,WAGNER);
  }

  // Fermeture de la grille de jeu (SDL)
  printf("(THREAD MAIN) Fermeture de la grille..."); fflush(stdout);
  FermerGrilleSDL();
  printf("OK\n"); fflush(stdout);

  exit(0);
}

