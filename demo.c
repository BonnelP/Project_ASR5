#include "socklib.h"
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
  int server = 0;
  int s;

  //var pour récup addr Client
  socklen_t len;
  struct sockaddr_storage addr;
  char ipstr[INET6_ADDRSTRLEN];
  int port;
  
  if (argc == 2) {
    // je suis le serveur (l'argument est le port)
    server = 1;

    
    
    // Création de la socket d'attente
    int sock_attente = CreeSocketServeur(argv[1]);
    if (sock_attente == -1) {
      exit(1);
    }

    // attente du client
    s = AcceptConnexion(sock_attente);
    
    //Recuperation @ client
    len = sizeof addr;
    getpeername(s, (struct sockaddr*)&addr, &len);

    // traiter IPv4 et IPv6:
    if (addr.ss_family == AF_INET) {
      struct sockaddr_in *s = (struct sockaddr_in *)&addr;
      port = ntohs(s->sin_port);
      inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    } else { // AF_INET6
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
      port = ntohs(s->sin6_port);
      inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }
    printf("Client IP address: %s\n", ipstr);
    printf("Client port      : %d\n", port);   

      
  } else if (argc == 3){
    // Je suis le client
    server = 0;

    GetIp();
    
    // création d'une socker et connexion
    s = CreeSocketClient(argv[1], argv[2]);
  } else {
    // il y a un problème car il manque d'argument
    fprintf(stderr, "Usage serveur: %s <port>\n       client: %s <server> <port>\n", argv[0], argv[0]);
    exit(1);
  }


  if (server) {

    
    
    // un message à envoyer
    const char *mess = " Telle est la réponse à la question ... ";

    // Envoie d'un premier message avec la taille de la suite
    // e premier message fait 30 caractères
    EnvoieMessage(s, "TailleMessage:%16d", strlen(mess));
    // Envoie d'un second message avec le reste
    EnvoieMessage(s, mess);


    
  } else {
    char buff[31];

    // lecure des 30 premiers caractères
    int r = recv(s, buff, 30, MSG_WAITALL);
    if (r == -1) {
      perror("recv");
    }
    // J'ajoute le caractère de fin de chaine à la fin du message recu
    buff[r] = '\0';
    fprintf(stdout, "Le client à recu '%s'\n", buff);

    // lecture de la taille du second message
    int taille;
    sscanf(buff, "TailleMessage:%16d", &taille);
    // lecure de la suite du message
    char buff2[taille];
    r = recv(s, buff2, taille, MSG_WAITALL);
    if (r == -1) {
      perror("recv");
    }
    
    // ecriture du message (comme un ensemble d'octet et pas comme une chaine de caractère)
    write(STDOUT_FILENO, buff2, r);
    fprintf(stdout, "\n");
    
  }
   
}
