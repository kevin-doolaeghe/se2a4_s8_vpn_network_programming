/**** Fichier principal pour le client du pont virtuel ****/

/** Fichiers d'inclusion **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include "libnet.h"

/** Quelques constantes **/
#define MACHINE "172.26.145.68"
#define PORT "http"
#define MAX_TAMPON 1024
#define NOM_INTERFACE "tap0"

/** Variables globales */

/** Fonctions **/

/* Fonction connexion serveur */
int connexionServeur(char *hote,char *service){
	struct addrinfo precisions,*resultat,*origine;
	int statut;
	int s;

	/* Creation de l'adresse de socket */
	memset(&precisions,0,sizeof precisions);
	precisions.ai_family=AF_UNSPEC;
	precisions.ai_socktype=SOCK_STREAM;
	statut=getaddrinfo(hote,service,&precisions,&origine);
	if(statut<0){ perror("connexionServeur.getaddrinfo"); exit(EXIT_FAILURE); }
	struct addrinfo *p;
	for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
		if(p->ai_family==AF_INET6){ resultat=p; break; }

	/* Creation d'une socket */
	s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
	if(s<0){ perror("connexionServeur.socket"); exit(EXIT_FAILURE); }

	/* Connexion de la socket a l'hote */
	if(connect(s,resultat->ai_addr,resultat->ai_addrlen)<0) return -1;

	/* Liberation de la structure d'informations */
	freeaddrinfo(origine);

	return s;
}

/* Fonction de communication avec le serveur */
void communicationServeur(int s){
	/* Boucle de communication avec le serveur */
	struct pollfd descripteurs[2];
	descripteurs[0].fd=s;
	descripteurs[0].events=POLLIN;
	descripteurs[1].fd=0;
	descripteurs[1].events=POLLIN;
	while(1){
		char tampon[MAX_TAMPON];
		int nb=poll(descripteurs,2,-1);
		if(nb<0){ perror("main.poll"); exit(EXIT_FAILURE); }
		if((descripteurs[0].revents&POLLIN)!=0){
			int taille=read(s,tampon,MAX_TAMPON);
			if(taille<=0) break;
			write(1,tampon,taille);
 		}
		if((descripteurs[1].revents&POLLIN)!=0){
			int taille=read(0,tampon,MAX_TAMPON);
			if(taille<=0) break;
			write(s,tampon,taille);
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
	int s=connexionServeur(MACHINE,PORT);
	if(s<0){ fprintf(stderr,"Erreur de connexion au serveur\n"); exit(EXIT_FAILURE); }

	// Ouverture de l'interface reseau
	int ret=creationInterfaceVirtuelle(NOM_INTERFACE);
	if(ret<0){
		fprintf(stderr,"Erreur de la création d'une interface virtuelle\n");
		exit(EXIT_FAILURE);
	}

	// Communication avec le serveur
	communicationServeur(s);

	return 0;
}
