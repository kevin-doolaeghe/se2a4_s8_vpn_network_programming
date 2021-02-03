/**** Fichier principal pour le pont virtuel ****/

/** Fichiers d'inclusion **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include "libnet.h"

/** Quelques constantes **/
#define SERVICE "http"
#define MAX_LIGNE 20

/** Variables globales **/

/** Fonctions **/

/* Fonction d'initialisation du serveur */
int initialisationServeur(char *service,int connexions){
	struct addrinfo precisions,*resultat,*origine;
	int statut;
	int s;

	/* Construction de la structure adresse */
	memset(&precisions,0,sizeof precisions);
	precisions.ai_family=AF_UNSPEC;
	precisions.ai_socktype=SOCK_STREAM;
	precisions.ai_flags=AI_PASSIVE;
	statut=getaddrinfo(NULL,service,&precisions,&origine);
	if(statut<0){ perror("initialisationServeur.getaddrinfo"); exit(EXIT_FAILURE); }

	struct addrinfo *p;
	for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
		if(p->ai_family==AF_INET6){ resultat=p; break; }

	/* Creation d'une socket */
	s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
	if(s<0){ perror("initialisationServeur.socket"); exit(EXIT_FAILURE); }

	/* Options utiles */
	int vrai=1;
	if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&vrai,sizeof(vrai))<0){
		perror("initialisationServeur.setsockopt (REUSEADDR)");
		exit(EXIT_FAILURE);
	}

	if(setsockopt(s,IPPROTO_TCP,TCP_NODELAY,&vrai,sizeof(vrai))<0){
		perror("initialisationServeur.setsockopt (NODELAY)");
		exit(EXIT_FAILURE);
	}

	/* Specification de l'adresse de la socket */
	statut=bind(s,resultat->ai_addr,resultat->ai_addrlen);
	if(statut<0) return -1;

	/* Liberation de la structure d'informations */
	freeaddrinfo(origine);

	/* Taille de la queue d'attente */
	statut=listen(s,connexions);
	if(statut<0) return -1;

	return s;
}

/* Fonction boucle serveur */
int boucleServeur(int ecoute,int (*traitement)(int)){
	int dialogue;

	while(1){
		/* Attente d'une connexion */
		if((dialogue=accept(ecoute,NULL,NULL))<0) return -1;

		/* Passage de la socket de dialogue a la fonction de traitement */
		if(traitement(dialogue)<0){ shutdown(ecoute,SHUT_RDWR); return 0;}
	}
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
	int s=initialisationServeur(SERVICE,MAX_CONNEXIONS);

	// Traitement des connexions et des messages
	boucleServeur(s,gestionClient);

	return 0;
}
