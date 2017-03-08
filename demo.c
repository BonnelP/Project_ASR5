#include "socklib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <fcntl.h>

#define TAILLE_MAX_NOMF 30
#define NBR_MAX_VOISIN 3
#define TAILLE_MAX_IP 15

//taille max du message du protocole = (option) + | + (taille_addresse) + | + ipsrc + | + taille_mess2 + | + mess2 +
//                                   :    1     + 1 +         2         + 1 +   15  + 1 +       2      + 1 +   45  +
#define TAILLE_MAX_MESS 69

void protocole(char *message, int option, size_t taille_mess2, char *mess2,char* ipsrc)
{
  //on concatène les différents parties du message à envoyer au serveur.
  // ipsrc peut etre  NULL
  // digit 1 : entier : option (1=insertion -- 2=recherche -- 0=Oquitter ...)
  sprintf(message,"%d",option);

  switch(option){
  case 1:
    // Si on choisit de s'inserer, le protocole sera comme ceci :
    // type : #(option)
	
    strcat(message, "|"); // ajout d'un pipe pour séparer
  case 2:
    // Si on choisit de chercher un fichier, le protocole sera comme ceci :
    // type : #(option)|##(taille_addresse)|ipsrc|##(taille_mess2)|mess2 (où # est un digit et | des séparateurs)
    
    strcat(message, "|"); 

    char tailleip[2];//stocke taille de l'ipsrc
    sprintf(tailleip,"d",strlen(ipsrc));
    strcat(message, tailleip);
    
    strcat(message, "|");
    
    strcat(message, ipsrc);
    
    strcat(message, "|");
    
    // 4 digit maximum pour la taille du rapport : entier
    char taille[2]; //pour stocker au plus 2 digit : la taille du nom du fichier
    sprintf(taille,"%lu",taille_mess2);
    strcat(message, taille);
      
    strcat(message, "|");
      
    strcat(message, mess2);
  case 3:
    // Si on repond a la demande d'insertion de le noeud, le protocole sera comme ceci:
    // type : #|##|#(full)|mess2
    // full=0 : le noeud possède au moins une place; =1 : le noeud est plein
    // mess2 contiendra quoiqu'il arrive la liste des voisins direct 

  }
	
}


void AffMenu()
{
  printf("\nMenu\n");
  printf("1 : Demande d'insertion dans le noeux\n");
  printf("2 : Demander un fichier\n");
  printf("0 : Quitter\n");
  printf("\n");
}


int main(int argc, char *argv[]) {
  int server = 0;
  int s;
  //int pid1;
  int resmenu;
  
  //var pour récup addr Client
  socklen_t len;
  struct sockaddr_storage addr;
  char ipstr[INET6_ADDRSTRLEN];
  int port;

  //fichier de stockage des adresses
  FILE* StockAddr = NULL;
  StockAddr = fopen("StockAddr.txt","a+");
  
  if (StockAddr == NULL)
    {
      // On affiche un message d'erreur si on veut
      printf("Impossible d'ouvrir le fichier StockAddr.txt");
    }
  
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
      fclose(StockAddr);
      exit(1);
    }
    
    while(1){
      
      // attente du client
      s = AcceptConnexion(sock_attente);
      if (s < 0){
	perror("ERREUR acceptCo");
	fclose(StockAddr);
	exit(1);
      }
      
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

	  
      char * test= RecoieLigne(s) ;
      write(STDOUT_FILENO,test,strlen(test));
    }
    fclose(StockAddr);
  }
  else {
    //On est le client
    char buff[31];
 
    char nomFile[31];
    // lecure des 30 premiers caractères

    AffMenu();
    char message[TAILLE_MAX_MESS];
    do{
      printf("\nQue choisissez-vous ? Tapez 1, 2 ou 0\n");
		
      scanf("%d",choix); //demande du choix

      switch(choix){
      case 0:
	printf("Quitter\n");
	
	break;
      case 1:
	printf("Vous avez choisi : Demande d'insertion dans le noeux \n\n");
	
	break;
      case 2:
	printf("Vous avez choisi : Demander un fichier \n\n");
	printf("Quel fichier voulez-vous? (30caractère max, evitez les espaces et caractères speciaux\n");
	scanf("%s",nomFile);
	
	break;
      default:
	printf("Choix non disponible. Recommencez.\n");
	
	break;
      }
    }while((choix>2));

    
    
    int r = recv(s, buff, 30, MSG_WAITALL);
    
    if (r == -1) {
      perror("recv client");
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
    
    // Envoie d'un message
    // EnvoieMessage(s, Saisi);
    fclose(StockAddr);
  }

}


  
