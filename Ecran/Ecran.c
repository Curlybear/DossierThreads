/* Exercice 1 :
   Gestion d'un fichier.
   In.Pr.E.S.
   Denys MERCENIER
   Ecran.c
*/

#include "Ecran.h"

void EffEcran(void)
{
printf("\033[2J");
fflush(stdout);
}

int LitChaine(char *pszChaine,int Lig,int Col,int Attrib,int Lg)
{
char        szBuffer[80];
int         L;

printf("\033[%d;%dH\033[%dm",Lig,Col,Attrib);
if (fgets(szBuffer,80,stdin) == NULL) return -1;
fflush(stdin);
L = strlen(szBuffer) - 1;
if (L >= Lg) L = Lg -1;

szBuffer[L] = '\0';
strncpy(pszChaine,szBuffer,80);
//pszChaine[strlen(pszChaine)-1] = 0;
printf("\033[%dm",NORM);
fflush(stdout);
return 0;
}

void AffChaine(char *pszChaine,int Lig,int Col, int Attrib)
{
printf("\033[%d;%dH\033[%dm%s\033[%dm",Lig,Col,Attrib,pszChaine,NORM);
fflush(stdout);
}

void Pause(void)
{
fflush(stdout);
getchar();
fflush(stdin);
}

void Trace(char *pszTrace, ... )
{
char szBuffer[80];
va_list arg;
va_start(arg,pszTrace);
vsprintf(szBuffer,pszTrace,arg);
fprintf(stderr,"%s\n",szBuffer);
va_end(arg);
return ;
}

void Affiche(char *pszTrace, ... )
{
char szBuffer[80];
va_list arg;
va_start(arg,pszTrace);
vsprintf(szBuffer,pszTrace,arg);
printf("%s\n",szBuffer);
fflush(stdout);
va_end(arg);
return ;
}

void SortieErreur(char *psz, int Rc)
{
perror(psz);
exit(Rc);
}

void RestitueCurseur()
{
printf("\0338");
fflush(stdout);
}

void SauveCurseur()
{
printf("\0337");
fflush(stdout);
}

void EffCadre(int Ligne,int Colonne,int Hauteur,int Largeur)
{
	char             szBuffer[1600];
	char             szLigneVide[81];
	int              i = 0;
	int              Pos;

	while (i < Largeur)
		    szLigneVide[i++] = ' ';

	szLigneVide[Largeur] = '\0';

	szBuffer[0] = '\0';
	i = 1;
	while (i <= Hauteur)
		   { Pos = strlen(szBuffer);
				      sprintf(&szBuffer[Pos],"\033[%d;%dH%s",Ligne + i,Colonne,szLigneVide);
							     i++;
		    }
	printf("%s",szBuffer);
	fflush(stdout);
}
