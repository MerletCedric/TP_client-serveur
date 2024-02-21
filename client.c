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
#include <time.h>
#include <unistd.h>
#include <ctype.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

#define NUM_WORDS 5
#define WORD_LENGTH 6
#define USERNAME_LENGTH (NUM_WORDS * (WORD_LENGTH + 1))

#define MAX_WORD_LENGTH 10
#define MAX_USERNAME_LENGTH 20

char* mots[] = {"chat", "soleil", "cascade", "forêt", "montagne", "rivière", "océan", "nuage", "étoile", "feu"};

char* generateUsername() {
    // Allocation de mémoire pour le pseudonyme
    char* pseudo = (char*)malloc(MAX_USERNAME_LENGTH + 1);
    if (pseudo == NULL) {
        printf("Erreur lors de l'allocation de mémoire.\n");
        exit(EXIT_FAILURE);
    }

    // Choix aléatoire de deux mots
    char mot1[MAX_WORD_LENGTH];
    char mot2[MAX_WORD_LENGTH];
    strcpy(mot1, mots[rand() % 10]);
    strcpy(mot2, mots[rand() % 10]);

    // Capitalisation des mots
    mot1[0] = toupper(mot1[0]);
    mot2[0] = toupper(mot2[0]);

    // Génération du pseudonyme en combinant les deux mots et un nombre aléatoire
    snprintf(pseudo, MAX_USERNAME_LENGTH + 1, "%s%s%d", mot1, mot2, rand() % 90 + 10);

    return pseudo;
}

int main(int argc, char **argv) {
 
    int longueur,       /* longueur d'un buffer utilisé */
        selection, /* Permet la sélection dans le menu */
        socket_dest; /* Socket du destinataire */
    sockaddr_in adresse_locale;     /* adresse de socket local */
    hostent * ptr_host;       /* info sur une machine hote */
    servent * ptr_service;        /* info sur service */
    char buffer[256];
    char * prog;           /* nom du programme */
    char * host;           /* nom de la machine distante */
    char msgToSend[1024];  /* messages envoyés */
    char msgToRsv[1024];   /* messages reçus */

    char username_dest[20];

    typedef struct {
        int socket;
        char username[20];
    } SOCKET;

    SOCKET socket_descriptor;  /* descripteur de socket */
        
    prog = argv[0];
    host = argv[1];

    const char LISTE_CLIENTS[20] = "liste_client",
        MSG_TOUS[20] = "msg_tous",
        MSG_GROUPE[20] = "msg_groupe",
        MSG_SEUL[20] = "msg_seul",
        CREER_GRP[20] = "creer_grp",
        REJOINDRE_GRP[20] = "rejoindre_grp",
        QUITTER[20] = "quitter";

    char choix[20]; /* Choix de menu retenu par l'utilisateur */

    srand(time(NULL));

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
    if ((socket_descriptor.socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le serveur.");
        exit(1);
    }

    strcpy(socket_descriptor.username,generateUsername()); // Générer un pseudonyme aléatoire

    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor.socket, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de se connecter au serveur.");
        exit(1);
    }

    write(socket_descriptor.socket, socket_descriptor.username, strlen(socket_descriptor.username));

    printf("\n--------------------------------------------\n");
    printf("Bienvenue %s\n", socket_descriptor.username);
    printf("--------------------------------------------\n");

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

        if ((write(socket_descriptor.socket, choix, strlen(choix))) < 0) {
            perror("erreur[Menu] : impossible d'envoyer un choix au serveur.");
            exit(1);
        }
        
        printf("Vous avez demandez : %s\n", choix);
        if((longueur = read(socket_descriptor.socket, buffer, sizeof(buffer))) > 0) {
            write(1, buffer, longueur);
            if (selection == 2) {
                printf("|i| Pour quitter le chat entrez : quitter\n");
            }
            while (selection == 2) {
                printf("> ");
                scanf("%s", buffer);
                if (strcmp(buffer, QUITTER) == 0) {
                    printf("|i| Vous avez quitté le chat\n");
                    selection = 7;
                    break;
                }
                write(socket_descriptor.socket, buffer, strlen(buffer));
                read(socket_descriptor.socket, buffer, strlen(buffer));
            }
        }
    } while(selection != 7);

    printf("\nfin de la reception.\n");
    close(socket_descriptor.socket);
    //free(username); // Libérer la mémoire allouée pour le pseudonyme
    printf("connexion avec le serveur fermee, fin du programme.\n");

    exit(0);
}
