#ifndef RESSOURCES_H
#define RESSOURCES_H

#include "GrilleSDL.h"

// Macros associees aux sprites
#define CHIFFRE_0                100000
#define CHIFFRE_1                100001
#define CHIFFRE_2                100002
#define CHIFFRE_3                100003
#define CHIFFRE_4                100004
#define CHIFFRE_5                100005
#define CHIFFRE_6                100006
#define CHIFFRE_7                100007
#define CHIFFRE_8                100008
#define CHIFFRE_9                100009

#define LETTRE_A                 200001
#define LETTRE_B                 200002
#define LETTRE_C                 200003
#define LETTRE_D                 200004
#define LETTRE_E                 200005
#define LETTRE_F                 200006
#define LETTRE_G                 200007
#define LETTRE_H                 200008
#define LETTRE_I                 200009
#define LETTRE_J                 200010
#define LETTRE_K                 200011
#define LETTRE_L                 200012
#define LETTRE_M                 200013
#define LETTRE_N                 200014
#define LETTRE_O                 200015
#define LETTRE_P                 200016
#define LETTRE_Q                 200017
#define LETTRE_R                 200018
#define LETTRE_S                 200019
#define LETTRE_T                 200020
#define LETTRE_U                 200021
#define LETTRE_V                 200022
#define LETTRE_W                 200023
#define LETTRE_X                 200024
#define LETTRE_Y                 200025
#define LETTRE_Z                 200026
#define LETTRE_ESPACE            200027

#define WAGNER                   300000
#define MERCENIER                300001
#define VILVENS                  300002
#define DEFOOZ                   300003
#define GERARD                   300004
#define CHARLET                  300005
#define MADANI                   300006

#define BRIQUE                   400000
#define FUSION                   400001
#define VOYANT_VERT              400002
#define VOYANT_BLEU              400003
#define VOYANT_ROUGE             400004

void ChargementImages();
void DessineChiffre(int L,int C,int chiffre);
void DessineLettre(int L,int C,char c);

#endif
