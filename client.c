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
    hostent * ptr_host;       /* info sur une machine hote */
    servent * ptr_service;        /* info sur service */
    char buffer[256];
    char * prog;           /* nom du programme */
    char * host;           /* nom de la machine distante */
    char * mesg;           /* message envoyé */

    prog = argv[0];
    host = argv[1];

    const char LISTE_CLIENTS[256] = "liste_client",
        MSG_TOUS[256] = "msg_tous",
        MSG_GROUPE[256] = "msg_groupe",
        MSG_SEUL[256] = "msg_seul",
        CREER_GRP[256] = "creer_grp",
        REJOINDRE_GRP[256] = "rejoindre_grp",
        QUITTER[256] = "quitter";

     char choix[256]; /* Choix de menu retenu par l'utilisateur */

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

    printf("\nChoisissez une catégorie :\n 1- Consulter les clients connectés \n 2- Envoyer un message à tous \n 3- Envoyer un message à un groupe d'utilisateur\n");
    printf(" 4- Envoyer un message à un utilisateur \n 5- Créer un groupe \n 6- Rejoindre un groupe \n 7- Quitter le chat \n");

    do {
        do {
            printf("\nMon choix : ");
            scanf("%d", &selection); 
            switch(selection) {
                case 1:
                    strcpy(choix, LISTE_CLIENTS);
                    break;
                case 2:
                    strcpy(choix, MSG_TOUS);
                    break;
                case 3:
                    strcpy(choix, MSG_GROUPE);
                    break;
                case 4:
                    strcpy(choix, MSG_SEUL);
                    break;
                case 5:
                    strcpy(choix, CREER_GRP);
                    break;
                case 6:
                    strcpy(choix, REJOINDRE_GRP);
                    break;
                case 7:
                    strcpy(choix, QUITTER);
                    break;
                default:
                    printf("\n /!\\ Veuilllez sélectionner un menu existant.\n");
            }
        } while (selection < 1 || selection > 7);
        if ((write(socket_descriptor, choix, strlen(choix))) < 0) {
            perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
            exit(1);
        }
        
        printf("Vous avez demandez : %s\n", choix);
        printf("Réponse du serveur : \n");
        while((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
            write(1,buffer,longueur);
        }
    } while(selection != 7);

    printf("\nfin de la reception.\n");
 
    close(socket_descriptor);

    printf("connexion avec le serveur fermee, fin du programme.\n");

    exit(0);
}
