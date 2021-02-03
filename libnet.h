/** Fichier libnet.h **/

/******************************************************************/
/** Ce fichier decrit les structures et les constantes utilisees **/
/** par les fonctions reseau.                                    **/
/******************************************************************/

/**** Constantes ****/

/** Nombre maximum de connexions tamponnees pour le serveur **/
#define MAX_CONNEXIONS	32

/**** Fonctions ****/

int connexionServeur(char *hote,char *service);
int initialisationServeur(char *service,int connexions);
int read_fixed(int descripteur,unsigned char *array,int size);
int creationInterfaceVirtuelle(char *nom);
