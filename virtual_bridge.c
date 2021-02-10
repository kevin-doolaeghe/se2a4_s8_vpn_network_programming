/**** Fichier principal pour le pont virtuel ****/

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
#define MAX_TAMPON 2048

/** Variables globales **/

/** Fonctions **/

/* Fonction boucle serveur */
int boucleServeur(int ecoute){
	int i,j;
	int nb_con=0;
	
  struct pollfd descripteurs[MAX_CONNEXIONS];
	descripteurs[0].fd=ecoute;
	descripteurs[0].events=POLLIN;
	for(i=1;i<MAX_CONNEXIONS;i++){
		descripteurs[i].fd=-1;
		descripteurs[i].events=POLLIN;
	}

  unsigned char tampon[MAX_TAMPON];
  unsigned short taille;
  int ret;

	while(nb_con>=0){
		int nb=poll(descripteurs,nb_con+1,-1);
		if(nb<0){ perror("main.poll"); exit(EXIT_FAILURE); }
		if((descripteurs[0].revents&POLLIN)!=0){
			// Récupère le nouveau socket
      int dialogue;
			if((dialogue=accept(ecoute,NULL,NULL))<0) return -1;

			// Sauvegarde du descripteur du socket si la connexion réussit
			nb_con++;
			descripteurs[nb_con].fd=dialogue;
 		}
		for(i=1;i<=nb_con;i++){
			if((descripteurs[i].revents&POLLIN)!=0){
				// Action à effectuer pour le socket correspondant
				ret=read_fixed(descripteurs[i].fd,(unsigned char *)&taille,sizeof(taille));
				if(ret<sizeof(taille)) break;
				ret=read_fixed(descripteurs[i].fd,tampon,taille); 
				if(ret!=taille) break;
				for(j=1;j<=nb_con;j++){
					// Pas d'envoi à soi-même
					if(j==i) continue;
					// Flag en cas de non réception (= supprimer le socket)
					unsigned char pb=0;
					ret=write(descripteurs[j].fd,(char *)&taille,sizeof(taille));
					if(ret!=sizeof(taille)) pb=1;
					ret=write(descripteurs[j].fd,tampon,taille);
					if(ret!=taille) pb=1;
					if(pb==1){
						// Décaler descripteur
						shutdown(descripteurs[j].fd,SHUT_RDWR);
						int k;
						for(k=j;k<=nb_con;k++){
							descripteurs[k].fd=descripteurs[k+1].fd;
						}
						nb_con--;
						// Repasse sur k (k+1 est devenu k)
						j--;
					}
       			}
			}
		}
	}

	for(i=1;i<MAX_CONNEXIONS;i++){
		shutdown(descripteurs[i].fd,SHUT_RDWR);
	}

	/* On termine la connexion */
	shutdown(ecoute,SHUT_RDWR);
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
	boucleServeur(s);

	return 0;
}
