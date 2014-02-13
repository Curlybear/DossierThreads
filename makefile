.SILENT:

GRILLESDL=GrilleSDL
RESSOURCES=Ressources
CLIENTTETRIS=ClientTetris
ECRAN=Ecran

CC = g++ -DKNOPPIX -I$(ECRAN) -I$(GRILLESDL) -I$(RESSOURCES) -I$(CLIENTTETRIS) -I /opt/guengel/include/SDL
OBJS = $(GRILLESDL)/GrilleSDL.o $(RESSOURCES)/Ressources.o $(CLIENTTETRIS)/ClientTetris.o $(ECRAN)/Ecran.o
PROGRAMS = Tetris Serveur

ALL: $(PROGRAMS)

Tetris:	Tetris.c $(OBJS)
	echo Creation de Tetris...
	$(CC) Tetris.c -o Tetris $(OBJS) -lrt -lpthread -lSDL

$(GRILLESDL)/GrilleSDL.o:	$(GRILLESDL)/GrilleSDL.c $(GRILLESDL)/GrilleSDL.h
		echo Creation de GrilleSDL.o ...
		$(CC) -c $(GRILLESDL)/GrilleSDL.c
		mv GrilleSDL.o $(GRILLESDL)

$(RESSOURCES)/Ressources.o:	$(RESSOURCES)/Ressources.c $(RESSOURCES)/Ressources.h
		echo Creation de Ressources.o ...
		$(CC) -c $(RESSOURCES)/Ressources.c
		mv Ressources.o $(RESSOURCES)

$(CLIENTTETRIS)/ClientTetris.o:	$(CLIENTTETRIS)/ClientTetris.c $(CLIENTTETRIS)/ClientTetris.h
		echo Creation de ClientTetris.o ...
		$(CC) -c $(CLIENTTETRIS)/ClientTetris.c
		mv ClientTetris.o $(CLIENTTETRIS)

$(ECRAN)/Ecran.o:	$(ECRAN)/Ecran.c $(ECRAN)/Ecran.h
		$(CC) $(ECRAN)/Ecran.c -c
		mv Ecran.o $(ECRAN)/Ecran.o


Serveur:	Serveur.c $(ECRAN)/Ecran.o
	echo Creation de Serveur...
	$(CC) Serveur.c $(ECRAN)/Ecran.o -o Serveur -lrt -lpthread

clean:
	@rm -f $(OBJS) core

clobber:	clean
	@rm -f tags $(PROGRAMS)
