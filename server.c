/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>  /* pour les sockets */
#include <sys/socket.h>
#include <netdb.h>    /* pour hostent, servent */
#include <string.h>     /* pour bcopy, ... */
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#define TAILLE_MAX_NOM 256
#define MAX_CLIENTS 4

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

typedef struct  {
  int socket;
  char username[20];
} SOCKET;

typedef struct {
  SOCKET sockets[MAX_CLIENTS];
  int count;
} ClientList;

 const char LISTE_CLIENTS[256] = "liste_client",
        MSG_TOUS[256] = "msg_tous",
        MSG_GROUPE[256] = "msg_groupe",
        MSG_SEUL[256] = "msg_seul",
        CREER_GRP[256] = "creer_grp",
        REJOINDRE_GRP[256] = "rejoindre_grp",
        QUITTER[256] = "quitter";

ClientList clientList;

// Fonction pour ajouter un client à la liste
void addClient(int socket, char *username) {
  if (clientList.count < MAX_CLIENTS) {
    clientList.sockets[clientList.count].socket = socket;
    strcpy(clientList.sockets[clientList.count].username, username);
    printf("[+] %s à été ajouté à la liste (Client #%d)\n", clientList.sockets[clientList.count].username, clientList.sockets[clientList.count].socket);
    clientList.count++;
  } else {
    printf("erreur[ajout client] : trop de client souhaite se connecter");
    close(socket);
  }
}

// Fonction pour supprimer un client de la liste
void removeClient(int socket) {
  int i;
  for (i = 0; i < clientList.count; i++) {
    if (clientList.sockets[i].socket == socket) {
      for (int j = i; j < clientList.count - 1; j++) {
          clientList.sockets[j] = clientList.sockets[j + 1];
      }
      clientList.count--;
      break;
    }
  }
  printf("[-] le client #%d s'est déconnecté\n", socket);
  close(socket);
}

// Fonction pour envoyer la liste des clients à un client spécifique
void sendClientList(int socket) {
  char buffer[256] = "";
  int i, length = 0;
  for (i = 0; i < clientList.count; i++) {
      length += sprintf(buffer + length, "Client %s\n", clientList.sockets[i].username);
  }
  write(socket, buffer, length);
}

void *discussToAll(int socket, char *buffer) {
  int longueur;
  char message_receiver[1024+30];
  char message_sender[1024+30];

  while ((longueur = read(socket, buffer, sizeof(buffer))) > 0) {
    printf("Message lu : %s\n", buffer);
    if (strcmp(buffer, QUITTER) == 0) {
      close(socket);
      pthread_exit(NULL);
    } else {
      buffer[longueur] = '\0';
      printf("Message \" %s \" : renvoyé a l'émetteur\n", buffer);

      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientList.sockets[i].socket != 0 && clientList.sockets[i].socket != socket) {
          sprintf(message_receiver, "%s : %s", clientList.sockets[socket].username, buffer);
          if (write(clientList.sockets[i].socket, message_receiver, strlen(message_receiver)) < 0) {
              perror("erreur [Discussion] : impossible d'écrire sur le descripteur de fichier.\n");
              close(clientList.sockets[i].socket);
              removeClient(clientList.sockets[i].socket);
          }
        } else if (clientList.sockets[i].socket == socket) {
          sprintf(message_sender, "Moi : %s\n", buffer);
          if (write(clientList.sockets[i].socket, message_sender, strlen(message_sender)) < 0) {
              perror("erreur [Discussion] : impossible d'écrire sur le descripteur de fichier.\n");
              close(clientList.sockets[i].socket);
              removeClient(clientList.sockets[i].socket);
          }
        }
      }
    }
  }
}

void *receptionChoix(void *currentClient_sock) {
  SOCKET *sock = (SOCKET *)currentClient_sock;
  int socket = sock->socket; /* récupération du socket client */
  char buffer[1024], msg[256];
  int longueur;

  // lecture du pseudonyme du client
  memset(buffer, 0, sizeof(buffer));
  if(read(socket, buffer, sizeof(buffer)) < 0){
    perror("erreur [Connexion] : impossible de lire le pseudonyme client. \n");
    close(socket);
    exit(1);
  };

  // Lorsqu'un client se connecte, ajouter son socket à la liste des clients
  addClient(socket, buffer);
  memset(buffer, 0, sizeof(buffer));

  while ((longueur = read(socket, buffer, sizeof(buffer))) > 0) {
    printf("\nChoix lu %s\n", buffer);

    if (strcmp(buffer, LISTE_CLIENTS) == 0) {
        sendClientList(socket);
        printf("Liste des clients envoyé au client #%d\n", socket);
    } else if (strcmp(buffer, MSG_TOUS) == 0) {
        memset(buffer, 0, sizeof(buffer));
        strcpy(msg, "---- CHAT ----\n");
        write(socket, msg, strlen(msg));
        discussToAll(socket, buffer); // Passer le socket émetteur
    } else if (strcmp(buffer, MSG_GROUPE) == 0) {
      printf("|i| Message à un groupe : en construction");
      strcpy(msg, "|i| Option encore indisponible\n");
      write(socket, msg, strlen(msg));
    } else if (strcmp(buffer, MSG_SEUL) == 0) {
      printf("|i| Message à un client : en construction");
      strcpy(msg, "|i| Option encore indisponible\n");
      write(socket, msg, strlen(msg));
    } else if (strcmp(buffer, CREER_GRP) == 0) {
      printf("|i| Créer un groupe : en construction");
      strcpy(msg, "|i| Option encore indisponible\n");
      write(socket, msg, strlen(msg));
    } else if (strcmp(buffer, REJOINDRE_GRP) == 0) {
      printf("|i| Rejoindre un groupe : en construction");
      strcpy(msg, "|i| Option encore indisponible\n");
      write(socket, msg, strlen(msg));
    } else if (strcmp(buffer, QUITTER) == 0) {
      removeClient(socket);
      break;
    } else {
      printf("erreur [reception incorrecte] : choix indisponible dans le menu.\n");
      strcpy(msg, "|i| Impossible de vous connecter pour le moment, veuillez patienter que d'autre utilisateurs se déconnecte \n");
      write(socket, msg, strlen(msg));
      close(socket);
      break;
    }
    //réinitialise le buffer
    memset(buffer, 0, sizeof(buffer));
  }
  free(currentClient_sock);
  pthread_exit(NULL);
}

// Exécution principale
main(int argc, char **argv) {
  int longueur_adresse_courante, /* longueur d'adresse courante d'un client */
    rc,  /* infos recuperees à la creation d'un thread */
    longueur,
    client_count = 0, /* compteur de client */
    port;
  sockaddr_in   adresse_locale,     /* structure d'adresse locale*/
    adresse_client_courant;   /* adresse client courant */
  hostent * ptr_hote;       /* les infos recuperees sur la machine hote */
  servent * ptr_service;      /* les infos recuperees sur le service de la machine */
  char machine[TAILLE_MAX_NOM+1];  /* nom de la machine locale */
  char buffer[256], maxClient_msg[256]; 
  longueur_adresse_courante = sizeof(adresse_client_courant);
  SOCKET socket_descriptor;   /* descripteur de socket */
  SOCKET *nouv_socket_descriptor; /* [nouveau] descripteur de socket */

  gethostname(machine,TAILLE_MAX_NOM);    /* recuperation du nom de la machine */

  /* recuperation de la structure d'adresse en utilisant le nom */
  if ((ptr_hote = gethostbyname(machine)) == NULL) {
    perror("erreur : impossible de trouver le serveur a partir de son nom.");
    exit(1);
  }

  /* copie de ptr_hote vers adresse_locale */
  bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
  adresse_locale.sin_family   = ptr_hote->h_addrtype;   /* ou AF_INET */
  adresse_locale.sin_addr.s_addr  = INADDR_ANY;       /* ou AF_INET */

  printf("Veuillez choisir un port d'attache : ");
  scanf("%d", &port);
  adresse_locale.sin_port = htons(port);

  printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);

  /* creation de la socket */
  if ((socket_descriptor.socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("erreur : impossible de creer la socket de connexion avec le client.");
    exit(1);
  }

  /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
  if ((bind(socket_descriptor.socket, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
    perror("erreur : impossible de lier la socket a l'adresse de connexion.");
    exit(1);
  }

  /* initialisation de la file d'ecoute */
  listen(socket_descriptor.socket,5);

  /* attente des connexions et traitement des donnees reçues */
  for (;;) {
    /* Allocation de la mémoire pour le nouveau socket */

    SOCKET *nouv_socket_descriptor = (SOCKET *)malloc(sizeof(SOCKET));
    if (nouv_socket_descriptor == NULL) {
        perror("erreur [Allocation mémoire] : impossible d'allouer la mémoire pour le socket client.\n");
        close(socket_descriptor.socket);
        exit(1);
    }

    /* adresse_client_courant sera renseignée par accept via les infos du connect */
    socklen_t longueur_adresse_courante = sizeof(adresse_client_courant);

    if ((nouv_socket_descriptor->socket =
            accept(socket_descriptor.socket,
                   (struct sockaddr *)(&adresse_client_courant),
                   &longueur_adresse_courante))
        < 0) {
        perror("erreur [Connexion] : impossible d'accepter la connexion avec le client. \n");
        close(socket_descriptor.socket);
        exit(1);
    }

    if (clientList.count >= MAX_CLIENTS) {
      printf("|i| Trop de connexion client.\n");
      close(nouv_socket_descriptor->socket);
      continue;
    }
    
    printf("\n> Connexion réussie avec client #%d! <\n\n", nouv_socket_descriptor->socket);

    pthread_t currentClient_thread;

    int rc = pthread_create(&currentClient_thread, NULL, receptionChoix, (void *)nouv_socket_descriptor);
    if (rc) {
        printf("erreur[creation thread] : creation du thread pour le client #%d\n, detail : %d", nouv_socket_descriptor->socket, rc);
        close(nouv_socket_descriptor->socket);
        free(nouv_socket_descriptor);
        //free(clientList);
        exit(1);
    }

    pthread_detach(currentClient_thread);
    //free(clientList);
    // Ne libérez pas nouveau_socket_descriptor ici car il est utilisé par le thread détaché.
  }
  close(socket_descriptor.socket);
  return 0;
}