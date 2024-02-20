/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

int main(int argc, char **argv) {
 
    int socket_descriptor,  /* descripteur de socket */
        longueur,       /* longueur d'un buffer utilisé */
        selection; /* Permet la sélection dans le menu */
    sockaddr_in adresse_locale;     /* adresse de socket local */
    hostent *   ptr_host;       /* info sur une machine hote */
    servent *   ptr_service;        /* info sur service */
    char buffer[256];
    char *  prog;           /* nom du programme */
    char *  host;           /* nom de la machine distante */
    char *  mesg;           /* message envoyé */

    prog = argv[0];
    host = argv[1];

    const char 
        LISTE_CLIENTS[256] = "liste_client",
        MSG_TOUS[256] = "msg_tous",
        MSG_GROUPE[256] = "msg_groupe",
        MSG_SEUL[256] = "msg_seul",
        CREER_GRP[256] = "creer_grp",
        REJOINDRE_GRP[256] = "rejoindre_grp",
        QUITTER[256] = "quitter";


    printf("nom de l'executable : %s \n", prog);
    printf("adresse du serveur  : %s \n", host);

    if ((ptr_host = gethostbyname(host)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }

    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */

    adresse_locale.sin_port = htons(5000);

    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));

    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le serveur.");
        exit(1);
    }

    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de se connecter au serveur.");
        exit(1);
    }

    printf("connexion etablie avec le serveur. \n");

    char choix[256];

    printf("Choisissez une catégorie :\n 1- Consulter les clients connectés \n 2- Envoyer un message à tous \n 3- Envoyer un message à un groupe d'utilisateur \n");
    printf("4- Envoyer un message à un utilisateur \n 5- Créer un groupe \n 6- Rejoindre un groupe \n 7- Quitter le chat \n Mon choix : ");
    scanf("%d", selection);

    do { 
        switch(selection) {
            case 1:
                if ((write(socket_descriptor, LISTE_CLIENTS, strlen(LISTE_CLIENTS))) < 0) {
                    perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
                    exit(1);
                }
            case 2:
                if ((write(socket_descriptor, MSG_TOUS, strlen(MSG_TOUS))) < 0) {
                    perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
                    exit(1);
                }
            case 3:
                if ((write(socket_descriptor, MSG_GROUPE, strlen(MSG_GROUPE))) < 0) {
                    perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
                    exit(1);
                }
            case 4:
                if ((write(socket_descriptor, MSG_SEUL, strlen(MSG_SEUL))) < 0) {
                    perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
                    exit(1);
                }
            case 5:
                if ((write(socket_descriptor, CREER_GRP, strlen(CREER_GRP))) < 0) {
                    perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
                    exit(1);
                }
            case 6:
                if ((write(socket_descriptor, REJOINDRE_GRP, strlen(REJOINDRE_GRP))) < 0) {
                    perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
                    exit(1);
                }
            case 7:
                if ((write(socket_descriptor, QUITTER, strlen(QUITTER))) < 0) {
                    perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
                    exit(1);
                }
        }
    } while (selection < 1 || selection > 5  );
    
    

    fgets(choix, sizeof(choix), stdin); // Demande à l'utilisateur de saisir le message
    choix[strcspn(choix, "\n")] = '\0'; // Supprime le caractère de nouvelle ligne ajouté par fgets

    if (strcmp(choix, "1") == 0) { // Comparaison avec strcmp()
        /* envoi du message vers le serveur */
        
        printf("requête envoyée. \n");
    } else if(strcmp(choix, "5") == 0) {
        /* envoi du message vers le serveur */
        if ((write(socket_descriptor, choix, strlen(choix))) < 0) {
            perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
            exit(1);
        }
    } else {
        printf("erreur[menu] : veuillez selectionner un menu disponible");
    }

    /* lecture de la reponse en provenance du serveur */
    sleep(3);
    printf("Voici les clients : ");
    while((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
        write(1,buffer,longueur);
    }

    printf("\nfin de la reception.\n");
 
    close(socket_descriptor);

    printf("connexion avec le serveur fermee, fin du programme.\n");

    exit(0);
}
