#include "socklib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  int server = 0;
  int s;
  // int finServ = 0;
  int pidClient;
  
  //var pour récup addr Client
  socklen_t len;
  struct sockaddr_storage addr;
  char ipstr[INET6_ADDRSTRLEN];
  int port;

  
  //fichier de stockage des adresses
  FILE* StockAddr = NULL;
  StockAddr = fopen("StockAddr.txt","a+");*/
  
    if (StockAddr == NULL)
      {
	// On affiche un message d'erreur si on veut
	printf("Impossible d'ouvrir le fichier StockAddr.txt");
      }
    */

  
  if (argc == 2) {
    // je suis le serveur (l'argument est le port)
    server = 1;  
  }
  else if (argc == 3){
    // Je suis le client
    server = 0;
    
    // création d'une socker et connexion
    s = CreeSocketClient(argv[1], argv[2]);
  }
  else {
    // il y a un problème car il manque d'argument
    fprintf(stderr, "Usage serveur: %s <port>\n       client: %s <server> <port>\n", argv[0], argv[0]);
    exit(1);
  }


  if (server) {
    
    // Création de la socket d'attente
    int sock_attente = CreeSocketServeur(argv[1]);
    if (sock_attente == -1) {
      exit(1);
    }
    
    while(1){
      
      // attente du client
      s = AcceptConnexion(sock_attente);
      if (s < 0){
	perror("ERREUR acceptCo");
	exit(1);
      }
      pidClient = fork();
      
      switch(pidClient){
      case -1:
	perror("ERREUR de fork client");
	exit(1);
      case 0:
	/*On est le fils*/
	close(s);

	//ici les actions a effectuer
	
	exit(0);
      default :
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
	printf("\nClient IP address: %s\n", ipstr);
	printf("Client port      : %d\n", port);   
	if (StockAddr != NULL)
	  {
	    fprintf(StockAddr,"%s\n",ipstr);
	  }
	
	// Envoie d'un premier message avec la taille de la suite
	EnvoieMessage(s, "TailleMessage:%16d", strlen(ipstr));
	// Envoie d'un second message avec le reste
	EnvoieMessage(s, ipstr);

	/*
	//Reception mess client
	char buff[31];
	//lecture 30 prem. caract. puis ajout caract. fin de chaine
	int r = recv(s,buff,30,MSG_WAITALL);
	if (r ==-1){
	  perror("recv serv");
	}
	buff[r] = '\0';
	fprintf(stdout,"Le serveur a recu '%s'\n",buff);
	//lecture 2nd mess
	int taille;
	sscanf(buff, "TailleMessage:%16d" ,&taille);
	//lecture suite
	char buff2[taille];
	r = recv(s,buff2,taille,MSG_WAITALL);
	if (r ==-1){
	  perror("recv serv");
	}

	//affichage du message en stdout
	write(STDOUT_FILENO,buff2,r);
	fprintf(stdout,"\n");
	*/
	char * test = RecoieLigne(s);
        write(STDOUT_FILENO,test,strlen(test));
	int RESD = RecoieEtSauveDonnees(fdStock,s);
	if(RESD == -1){
	  fclose(StockAddr);
	}
	else{
	  printf("\n%d octet lu\n",RESD);
	}
	close(s);	
      }
      fclose(StockAddr);
      close(fdStock);
    }
  } else {
    char buff[31];

    char Saisi[31];
    Saisi[0] = '\0';
    printf("A tous moment, saisissez FIN pour mettre arreter le programme \nVotre saisi ne peut exceder 30 caractere\n");
    // lecure des 30 premiers caractères
    
    int r = recv(s, buff, 30, MSG_WAITALL);
    
    if (r == -1) {
      perror("recv client");
    }
    if (r == 0 ){
      printf("Attendez votre tours svp,un client communique deja avec le serveur");
    }
    // J'ajoute le caractère de fin de chaine à la fin du message recu
    buff[r] = '\0';
    fprintf(stdout, "Le client a recu '%s'\n", buff);
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
    
    fprintf(stdout,"\nEntrez le nom du fichier que vous cherchez\n");
    scanf("%s",Saisi);
    
    // Envoie d'un premier message avec la taille de la suite
    //EnvoieMessage(s, "TailleMessage:%16d", strlen(Saisi));
    // Envoie d'un second message avec le reste
    EnvoieMessage(s, Saisi);
    
  }
  fclose(StockAddr);
  close(fdStock);
}
