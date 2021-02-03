/**** Fichier principal pour le client du pont virtuel ****/

/** Fichiers d'inclusion **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

#include "libnet.h"

/** Quelques constantes **/
#define MAX_TAMPON 1024
#define NOM_INTERFACE "tap0"

/** Variables globales */

/** Fonctions **/

/* Fonction de communication avec le serveur */
void communicationServeur(int s,int fd){
	/* Boucle de communication avec le serveur */
	struct pollfd descripteurs[2];
	descripteurs[0].fd=s;
	descripteurs[0].events=POLLIN;
	descripteurs[1].fd=fd; // File descriptor de l'interface virrtuelle
	descripteurs[1].events=POLLIN;
	while(1){
		char tampon[MAX_TAMPON];
		int nb=poll(descripteurs,2,-1);
		if(nb<0){ perror("main.poll"); exit(EXIT_FAILURE); }
		if((descripteurs[0].revents&POLLIN)!=0){
			// Interface réseau
			int taille=read(s,tampon,MAX_TAMPON);
			if(taille<=0) break;
			write(1,tampon,taille);
 		}
		if((descripteurs[1].revents&POLLIN)!=0){
			// Interface virtuelle

		}
	}

	/* On termine la connexion */
	shutdown(s,SHUT_RDWR);
}

/* Fonction principale */
int main(int argc,char *argv[]){
	// Analyse des arguments
	if(argc!=3){
		fprintf(stderr,"Syntaxe : client <serveur> <port>\n");
		exit(-1);
	}
	char *serveur=argv[1];
	char *service=argv[2];
	#ifdef DEBUG
	fprintf(stdout,"Pont sur %s port %s\n",serveur,service);
	#endif

	// Connexion au serveur
	int s=connexionServeur(serveur,service);
	if(s<0){ fprintf(stderr,"Erreur de connexion au serveur\n"); exit(EXIT_FAILURE); }

	// Ouverture de l'interface reseau
	int fd=creationInterfaceVirtuelle(NOM_INTERFACE);
	if(fd<0){
		fprintf(stderr,"Erreur de la création d'une interface virtuelle\n");
		exit(EXIT_FAILURE);
	}

	// Communication avec le serveur
	communicationServeur(s,fd);

	return 0;
}
