/**** Fichier principal pour le client du pont virtuel ****/

/** Fichiers d'inclusion **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "libnet.h"

/** Quelques constantes **/
#define MAX_TAMPON 2048
#define MAX_NOM_INTERFACE 16
#define NOM_INTERFACE "tap0"

/** Variables globales */

/** Fonctions **/

/* Fonction de communication avec le serveur */
void communicationServeur(int s, int fd)
{
    /* Boucle de communication avec le serveur */
    struct pollfd descripteurs[2];
    descripteurs[0].fd = s;
    descripteurs[0].events = POLLIN;
    descripteurs[1].fd = fd; // File descriptor de l'interface virrtuelle
    descripteurs[1].events = POLLIN;
    while (1) {
        unsigned char tampon[MAX_TAMPON];
        unsigned short taille;
        int ret;
        int nb = poll(descripteurs, 2, -1);
        if (nb < 0) {
            perror("main.poll");
            exit(EXIT_FAILURE);
        }
        if ((descripteurs[0].revents & POLLIN) != 0) {
            // Interface réseau
            ret = read_fixed(s, (unsigned char*)&taille, sizeof(taille));
            if (ret < sizeof(taille))
                break;
            ret = read_fixed(s, tampon, taille);
            if (ret != taille)
                break;
            write(fd, tampon, taille);
        }
        if ((descripteurs[1].revents & POLLIN) != 0) {
            // Interface virtuelle
            ret = read(fd, tampon, MAX_TAMPON);
            unsigned short tmp = ret;
            unsigned char pb = 0;
            ret = write(s, &tmp, sizeof(tmp));
            if (ret != sizeof(tmp))
                pb = 1;
            ret = write(s, tampon, tmp);
            if (ret != tmp)
                pb = 1;
            if (pb == 1) {
                break;
            }
        }
    }

    /* On termine la connexion */
    shutdown(s, SHUT_RDWR);
}

/* Fonction principale */
int main(int argc, char* argv[])
{
    // Analyse des arguments
    if (argc != 3) {
        fprintf(stderr, "Syntaxe : client <serveur> <port>\n");
        exit(-1);
    }
    char* serveur = argv[1];
    char* service = argv[2];
#ifdef DEBUG
    fprintf(stdout, "Pont sur %s port %s\n", serveur, service);
#endif

    // Connexion au serveur
    int s = connexionServeur(serveur, service);
    if (s < 0) {
        fprintf(stderr, "Erreur de connexion au serveur\n");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    fprintf(stdout, "Connexion au serveur effectuée\n");
#endif

    // Ouverture de l'interface reseau
    char interface[MAX_NOM_INTERFACE] = NOM_INTERFACE;
    int fd = creationInterfaceVirtuelle(interface);
    if (fd < 0) {
        fprintf(stderr, "Erreur de la création d'une interface virtuelle\n");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    fprintf(stdout, "Interface virtuelle créée\n");
#endif

    // Communication avec le serveur
    communicationServeur(s, fd);

    return 0;
}
