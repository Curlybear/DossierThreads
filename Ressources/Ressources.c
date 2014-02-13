#include "Ressources.h"

void ChargementImages()
{
  // Definition des sprites et de l'image de fond
  DessineImageFond("./images/FondEcran.bmp");

  // Sprites Profs
  AjouteSpriteAFondTransparent(VILVENS,"./images/profs/Vilvens40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(WAGNER,"./images/profs/Wagner40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(MERCENIER,"./images/profs/Mercenier40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(GERARD,"./images/profs/Gerard40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(DEFOOZ,"./images/profs/Defooz40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHARLET,"./images/profs/Charlet40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(MADANI,"./images/profs/Madani40.bmp",255,255,255);

  // Sprites Chiffres
  AjouteSpriteAFondTransparent(CHIFFRE_0,"./images/chiffres/Zero.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_1,"./images/chiffres/Un.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_2,"./images/chiffres/Deux.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_3,"./images/chiffres/Trois.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_4,"./images/chiffres/Quatre.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_5,"./images/chiffres/Cinq.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_6,"./images/chiffres/Six.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_7,"./images/chiffres/Sept.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_8,"./images/chiffres/Huit.bmp",255,255,255);
  AjouteSpriteAFondTransparent(CHIFFRE_9,"./images/chiffres/Neuf.bmp",255,255,255);

  // Sprites Lettres
  AjouteSpriteAFondTransparent(LETTRE_A,"./images/lettres/A.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_B,"./images/lettres/B.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_C,"./images/lettres/C.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_D,"./images/lettres/D.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_E,"./images/lettres/E.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_F,"./images/lettres/F.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_G,"./images/lettres/G.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_H,"./images/lettres/H.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_I,"./images/lettres/I.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_J,"./images/lettres/J.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_K,"./images/lettres/K.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_L,"./images/lettres/L.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_M,"./images/lettres/M.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_N,"./images/lettres/N.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_O,"./images/lettres/O.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_P,"./images/lettres/P.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_Q,"./images/lettres/Q.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_R,"./images/lettres/R.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_S,"./images/lettres/S.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_T,"./images/lettres/T.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_U,"./images/lettres/U.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_V,"./images/lettres/V.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_W,"./images/lettres/W.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_X,"./images/lettres/X.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_Y,"./images/lettres/Y.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_Z,"./images/lettres/Z.bmp",255,255,255);
  AjouteSpriteAFondTransparent(LETTRE_ESPACE,"./images/lettres/Espace.bmp",255,255,255);

  // Autres
  AjouteSpriteAFondTransparent(BRIQUE,"./images/Brique40.bmp",255,255,255);
  AjouteSpriteAFondTransparent(FUSION,"./images/Fusion40.bmp",255,255,255);
  AjouteSprite(VOYANT_VERT,"./images/VoyantVert40.bmp");
  AjouteSprite(VOYANT_BLEU,"./images/VoyantBleu40.bmp");
  AjouteSprite(VOYANT_ROUGE,"./images/VoyantRouge40.bmp");
}

void DessineChiffre(int L,int C,int chiffre)
{
  switch(chiffre)
  {
    case 0 : DessineSprite(L,C,CHIFFRE_0); break;
    case 1 : DessineSprite(L,C,CHIFFRE_1); break;
    case 2 : DessineSprite(L,C,CHIFFRE_2); break;
    case 3 : DessineSprite(L,C,CHIFFRE_3); break;
    case 4 : DessineSprite(L,C,CHIFFRE_4); break;
    case 5 : DessineSprite(L,C,CHIFFRE_5); break;
    case 6 : DessineSprite(L,C,CHIFFRE_6); break;
    case 7 : DessineSprite(L,C,CHIFFRE_7); break;
    case 8 : DessineSprite(L,C,CHIFFRE_8); break;
    case 9 : DessineSprite(L,C,CHIFFRE_9); break;
    default : break;
  }
}

void DessineLettre(int L,int C,char c)
{
  c = toupper(c);
  switch(c)
  {
    case 'A' : DessineSprite(L,C,LETTRE_A); break;
    case 'B' : DessineSprite(L,C,LETTRE_B); break;
    case 'C' : DessineSprite(L,C,LETTRE_C); break;
    case 'D' : DessineSprite(L,C,LETTRE_D); break;
    case 'E' : DessineSprite(L,C,LETTRE_E); break;
    case 'F' : DessineSprite(L,C,LETTRE_F); break;
    case 'G' : DessineSprite(L,C,LETTRE_G); break;
    case 'H' : DessineSprite(L,C,LETTRE_H); break;
    case 'I' : DessineSprite(L,C,LETTRE_I); break;
    case 'J' : DessineSprite(L,C,LETTRE_J); break;
    case 'K' : DessineSprite(L,C,LETTRE_K); break;
    case 'L' : DessineSprite(L,C,LETTRE_L); break;
    case 'M' : DessineSprite(L,C,LETTRE_M); break;
    case 'N' : DessineSprite(L,C,LETTRE_N); break;
    case 'O' : DessineSprite(L,C,LETTRE_O); break;
    case 'P' : DessineSprite(L,C,LETTRE_P); break;
    case 'Q' : DessineSprite(L,C,LETTRE_Q); break;
    case 'R' : DessineSprite(L,C,LETTRE_R); break;
    case 'S' : DessineSprite(L,C,LETTRE_S); break;
    case 'T' : DessineSprite(L,C,LETTRE_T); break;
    case 'U' : DessineSprite(L,C,LETTRE_U); break;
    case 'V' : DessineSprite(L,C,LETTRE_V); break;
    case 'W' : DessineSprite(L,C,LETTRE_W); break;
    case 'X' : DessineSprite(L,C,LETTRE_X); break;
    case 'Y' : DessineSprite(L,C,LETTRE_Y); break;
    case 'Z' : DessineSprite(L,C,LETTRE_Z); break;
    case ' ' : DessineSprite(L,C,LETTRE_ESPACE); break;
    case '0' : DessineSprite(L,C,CHIFFRE_0); break;
    case '1' : DessineSprite(L,C,CHIFFRE_1); break;
    case '2' : DessineSprite(L,C,CHIFFRE_2); break;
    case '3' : DessineSprite(L,C,CHIFFRE_3); break;
    case '4' : DessineSprite(L,C,CHIFFRE_4); break;
    case '5' : DessineSprite(L,C,CHIFFRE_5); break;
    case '6' : DessineSprite(L,C,CHIFFRE_6); break;
    case '7' : DessineSprite(L,C,CHIFFRE_7); break;
    case '8' : DessineSprite(L,C,CHIFFRE_8); break;
    case '9' : DessineSprite(L,C,CHIFFRE_9); break;
    default :  DessineSprite(L,C,LETTRE_ESPACE); break;
  }
}
