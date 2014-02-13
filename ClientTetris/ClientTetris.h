#ifndef CLIENT_TETRIS_H
#define CLIENT_TETRIS_H
#include <sys/msg.h>

typedef struct
{
  char  login[20];
  char  pseudo[80];
  int   score;  
} TOPSCORE;

int ConnectionServeur(key_t cle,const char* pseudo);
int EnvoiScore(key_t cle,int score);
int GetNbJoueursConnectes(key_t cle);
int GetTopScore(key_t cle,TOPSCORE *pTopScore);
int DeconnectionServeur(key_t cle);

#endif
