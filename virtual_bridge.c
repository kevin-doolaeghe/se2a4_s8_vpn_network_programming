/**** Fichier principal pour le pont virtuel ****/

/** Fichiers d'inclusion **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

#include "libnet.h"

/** Quelques constantes **/
#define MAX_LIGNE 20

/** Variables globales **/

/** Fonctions **/

/* Fonction boucle serveur */
int boucleServeur(int ecoute,int (*traitement)(int)){
	int dialogue;
	int i;
	int nb_con=0;

	while(1){
		/* Attente d'une connexion */
		if((dialogue=accept(ecoute,NULL,NULL))<0) return -1;

		/* Passage de la socket de dialogue a la fonction de traitement */
		if(traitement(dialogue)<0){ shutdown(ecoute,SHUT_RDWR); return 0;}
	}

	struct pollfd descripteurs[MAX_CONNEXIONS];
	descripteurs[0].fd=ecoute;
	descripteurs[0].events=POLLIN;
	for(i=1;i<MAX_CONNEXIONS;i++){
		descripteurs[i].fd=0;
		descripteurs[i].events=POLLIN;
	}

	while(nb_con>=0){
		int nb=poll(descripteurs,MAX_CONNEXIONS,-1);
		if(nb<0){ perror("main.poll"); exit(EXIT_FAILURE); }
		if((descripteurs[0].revents&POLLIN)!=0){
			// Récupère le nouveau socket
			int fd;
			if((fd=accept(ecoute,NULL,NULL))<0) return -1;

			// Sauvegarde du descripteur du socket si la connexion réussit
			nb_con++;
			descripteurs[nb_con].fd=fd;
 		}
		for(i=1;i<nb_con;i++){
			if((descripteurs[i].revents&POLLIN)!=0){
				// Action à effectuer pour le socket correspondant

			}
		}
	}

	for(i=1;i<MAX_CONNEXIONS;i++){
		shutdown(descripteurs[i].fd,SHUT_RDWR);
	}

	/* On termine la connexion */
	shutdown(ecoute,SHUT_RDWR);
}

/* Fonction gestion client */
int gestionClient(int s){
	/* Obtient une structure de fichier */
	FILE *dialogue=fdopen(s,"a+");
	if(dialogue==NULL){ perror("gestionClient.fdopen"); exit(EXIT_FAILURE); }

	/* Echo */
	char ligne[MAX_LIGNE];
	while(fgets(ligne,MAX_LIGNE,dialogue)!=NULL)
		fprintf(dialogue,"> %s",ligne);

	/* Termine la connexion */
	fclose(dialogue);
	return 0;
}

/* Fonction principale */
int main(int argc,char *argv[]){
	// Analyse des arguments
	if(argc!=2){
		fprintf(stderr,"Syntaxe : bridge <port>\n");
		exit(-1);
	}
	char *service=argv[1];
	#ifdef DEBUG
	fprintf(stdout,"Port : %s\n",service);
	#endif

	// Initialisation du serveur
	int s=initialisationServeur(service,MAX_CONNEXIONS);

	// Traitement des connexions et des messages
	boucleServeur(s,gestionClient);

	return 0;
}
