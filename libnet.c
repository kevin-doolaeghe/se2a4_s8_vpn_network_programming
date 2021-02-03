/** Fichier libnet.c **/

/************************************************************/
/** Ce fichier contient des fonctions reseau.              **/
/************************************************************/

/**** Fichiers d'inclusion ****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include "libnet.h"

/**** Constantes ****/

#define TAP_PRINCIPAL	"/dev/net/tun"

/**** Variables globales *****/

/**** Fonctions de gestion des sockets ****/

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

/** Lecture fixée **/
int read_fixed(int descripteur,unsigned char *array,int size){
  int bytes=0;
  while(bytes<size){
    int offset=read(descripteur,array+bytes,size-bytes);
    if(offset<=0) return -1; else bytes += offset;
  }
  return bytes;
}

/**** Fonctions de gestion des interfaces virtuelles ****/

/** Ouverture d'une interface Ethernet virtuelle **/
int creationInterfaceVirtuelle(char *nom){
  struct ifreq interface;
  int fd,erreur;

  /* Ouverture du peripherique principal */
  if((fd=open(TAP_PRINCIPAL,O_RDWR))<0) return fd;

  /* Preparation de la structure */
  memset(&interface,0,sizeof(interface));
  interface.ifr_flags=IFF_TAP|IFF_NO_PI;
  if(nom!=NULL) strncpy(interface.ifr_name,nom,IFNAMSIZ);

  /* Creation de l'interface */
  if((erreur=ioctl(fd,TUNSETIFF,(void *)&interface))<0){ close(fd); return erreur; }

  /* Recuperation du nom de l'interface */
  if(nom!=NULL) strcpy(nom,interface.ifr_name);

  return fd;
}
