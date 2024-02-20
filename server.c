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
#define TAILLE_MAX_NOM 256
#define MAX_CLIENTS 10

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;
typedef struct {
  int sockets[MAX_CLIENTS];
  int count;
} ClientList;
typedef struct  {
  int socket;
  char username[256];
} SOCKET;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

ClientList clientList;

// Fonction pour ajouter un client à la liste
void addClient(int socket) {
  if (clientList.count < MAX_CLIENTS) {
    clientList.sockets[clientList.count++] = socket;
    printf("[+] Le client #%d à été ajouté à la liste \n", socket);
  } else {
    printf("erreur[ajout client] : trop de client souhaite se connecter");
  }
}

// Fonction pour supprimer un client de la liste
void removeClient(int socket) {
  int i;
  for (i = 0; i < clientList.count; i++) {
    if (clientList.sockets[i] == socket) {
      for (int j = i; j < clientList.count - 1; j++) {
          clientList.sockets[j] = clientList.sockets[j + 1];
      }
      clientList.count--;
      break;
    }
  }
  printf("[-] le client #%d s'est déconnecté", socket);
}

// Fonction pour envoyer la liste des clients à un client spécifique
void sendClientList(int socket) {
  char buffer[256];
  int i, length = 0;
  for (i = 0; i < clientList.count; i++) {
      length += sprintf(buffer + length, "Client %d\n", clientList.sockets[i]);
  }
  write(socket, buffer, strlen(buffer));
  printf("Liste des clients envoyé au client #%d", socket);
}

void *receptionChoix(void *currentClient_sock) {
  
  SOCKET *sock = (SOCKET *)currentClient_sock;
  int socket = sock->socket; /* récupération du socket client */
  char buffer[256] = "";
  int longueur;

  if ((longueur = read(socket, buffer, sizeof(buffer))) <= 0) {
    printf("error : rien n'est lu");
    pthread_exit(NULL);
  }

  printf("Choix lu : %s \n", buffer);

  // Redirection des requêtes
  if (strcmp(buffer, "1") == 0) {
      sendClientList(socket);
  } else if (strcmp(buffer, "5") == 0) {
      removeClient(socket);
      close(socket);
      free(currentClient_sock);
      pthread_exit(NULL);
  }
}

// Exécution principale
main(int argc, char **argv) {
  int longueur_adresse_courante, /* longueur d'adresse courante d'un client */
    clients_count, /* Compteur de client qui se connecte */
    rc;  /* infos recuperees à la creation d'un thread */
  sockaddr_in   adresse_locale,     /* structure d'adresse locale*/
    adresse_client_courant;   /* adresse client courant */
  hostent * ptr_hote;       /* les infos recuperees sur la machine hote */
  servent * ptr_service;      /* les infos recuperees sur le service de la machine */
  char machine[TAILLE_MAX_NOM+1];  /* nom de la machine locale */
  char buffer[256]; 
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

  // printf("Veuillez choisir un port d'attache : ");
  // scanf("%d", &port);
  adresse_locale.sin_port = htons(5000);

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
        close(socket_descriptor);
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
        close(socket_descriptor);
        exit(1);
    }

    pthread_t currentClient_thread;

    /* Allocation de la mémoire pour la liste des clients */
    ClientList *clientList = (ClientList *)malloc(sizeof(ClientList));
    if (clientList == NULL) {
        perror("erreur : impossible d'allouer la mémoire pour les arguments de thread.");
        exit(1);
    }

    printf("> Connexion réussie avec client #%d! <\n\n", nouv_socket_descriptor->socket);

    //printf("test %p \n", (void *)nouv_socket_descriptor->socket);
    // Lorsqu'un client se connecte, ajouter son socket à la liste des clients
    addClient(nouv_socket_descriptor->socket);
    clients_count++;

    printf("Création du thread pour le socket #%d\n", nouv_socket_descriptor->socket);
    int rc = pthread_create(&currentClient_thread, NULL, receptionChoix, (void *)nouv_socket_descriptor);
    if (rc) {
        printf("erreur[creation thread] : creation du thread pour le client #%d\n, detail : %d", nouv_socket_descriptor->socket, rc);
        close(nouv_socket_descriptor->socket);
        free(nouv_socket_descriptor);
        free(clientList);
        exit(-1);
    }

    pthread_detach(currentClient_thread);
    
    close(socket_descriptor);
    // Ne libérez pas nouveau_socket_descriptor ici car il est utilisé par le thread détaché.
}
  return 0;
}