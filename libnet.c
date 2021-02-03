/** fichier libnet.c **/

/************************************************************/
/** Ce fichier contient des fonctions reseau.              **/
/************************************************************/

/**** Fichiers d'inclusion ****/

#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "libnet.h"

/**** Constantes ****/

#define TAP_PRINCIPAL	"/dev/net/tun"

/**** Variables globales *****/

/**** Fonctions de gestion des sockets ****/

/**** Fonctions de gestion des interfaces virtuelles ****/

/** Lecture fixée **/
int read_fixed(int descripteur,unsigned char *array,int size){
int bytes=0;
while(bytes<size){
  int offset=read(descripteur,array+bytes,size-bytes);
  if(offset<=0) return -1; else bytes += offset;
  }
return bytes;
}

/** Ouverture d'une interface Ethernet virtuelle **/

int creationInterfaceVirtuelle(char *nom)
{
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
