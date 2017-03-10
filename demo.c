#include "socklib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <fcntl.h>

#define NBR_MAX_VOISINS 3

#define TAILLE_MAX_NOMF 30
#define TAILLE_MAX_IP 15


//taille max du message du protocole = (option) + | + (taille_addresse) + | + ipsrc + | + taille_mess2 + | + mess2 +
//                                   :    1     + 1 +         2         + 1 +   15  + 1 +       2      + 1 +   45  +
#define TAILLE_MAX_MESS 69

#define OPT_EXI 0 //option pour fermer le client
#define OPT_INS	1
#define OPT_FIC 2
#define OPT_CLO 3 // option pour fermer le client ET le serveur , utile principalement pour les tests*
#define REP_INS 4

void protocole(char *message, int option, size_t taille_mess2, char *mess2,char *ipsrc)
{
  //Nous utilisons des pipe pour séparer les divers elements pour une lisibilite accru, il faudra par la suite les enlevers car non necessaire(nous connaissons l'agencement et la taille des diferrentes parties!

   char tailleip[2];//stocke taille de l'ipsrc
  
  //on concatène les différents parties du message à envoyer au serveur.
  // taille_mess2,mess2 et ipsrc peuvent etres  NULL ou 0
  // digit 1 : entier : option (1=insertion -- 2=recherche -- 0=Oquitter ...)
  sprintf(message,"%d",option);

  switch(option){
  case 1:
    // Si on choisit de s'inserer, le protocole sera comme ceci :
    // type : #(option)| 	
    break;
    
  case 2:
    // Si on choisit de chercher un fichier, le protocole sera comme ceci :
    // type : #(option)|##(taille_addresse)|ipsrc(7 à 15)|##(taille_mess2)|mess2 (où # est un digit et | des séparateurs)      

    if (strcmp(ipsrc,"") != 0){ 
      sprintf(tailleip,"%lu",strlen(ipsrc));
    }
    else {
      strcpy(tailleip,"07");
    }      
    strcat(message, tailleip);    
    if (strcmp(ipsrc,"")!=0){ 
      strcat(message, ipsrc);
    }
    else {
      strcat(message,"0.0.0.0"); //s'il n'y a pas d'ipsrc on met 0.0.0.0 (qui ne peut pas etre une ip src)
    }   
   
    // 4 digit maximum pour la taille du rapport : entier
    char taille[2]; //pour stocker au plus 2 digit : la taille du nom du fichier
    sprintf(taille,"%lu",taille_mess2);
    strcat(message, taille);     
    strcat(message, mess2);
    break;
    
  case 3:
    //fermerture du Client et du Serveur
    break;
    
  case 4:
    // Si on repond a la demande d'insertion de le noeud, le protocole sera comme ceci:
    // type : #|##|#(full)|mess2
    // full=0 : le noeud possède au moins une place; =1 : le noeud est plein
    // mess2 contiendra quoiqu'il arrive la liste des voisins direct 
    break;
  }
	
}


void AffMenu()
{
  printf("\nMenu\n");
  printf("1 : Demande d'insertion dans le noeux\n");
  printf("2 : Demander un fichier\n");
  printf("3 : Quitter la demo (!!Ferme egalement le serveur!!)\n");
  printf("0 : fermer le client\n");
  printf("\n");
}


      

char *decoupeStr (const char *src , int ind1 , int ind2)
{
  char *new_s = NULL;
  //ind1 donne l'indice du debut de la chaine a extraire et ind2 donne la fin
  
  if (src != NULL && ind1 <= ind2)
    {
      new_s = malloc (sizeof (*new_s) * (ind2 - ind1 + 2));
      if (new_s != NULL)
	{
	  int i;

	  for (i = ind1; i <= ind2; i++)
	    {
	      new_s[i-ind1] = src[i];
	    }
	  new_s[i-ind1] = '\0';
	}
      else
	{
	  fprintf (stderr, "new_s n'a pas ete alloue en memoire\n");
	  exit (EXIT_FAILURE);
	}
    }
  return new_s;
  free(new_s);
}




int main(int argc, char *argv[]) {
  int server = 0;
  int s;
  int tailleEnvoye;
  int nbrVoisins = 0;
  
  //var pour récup addr Client
  socklen_t len;
  struct sockaddr_storage addr;
  char ipstr[INET6_ADDRSTRLEN];
  int port;

  //fichier de stockage des adresses
  FILE* StockAddrServ = NULL;
  StockAddrServ = fopen("StockAddrServ.txt","a+");
  
  if (StockAddrServ == NULL)
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
      fclose(StockAddrServ);
      exit(1);
    }
    
    while(1){
      
      // attente du client
      s = AcceptConnexion(sock_attente);
      if (s < 0){
	perror("ERREUR acceptCo");
	fclose(StockAddrServ);
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
      
      if (StockAddrServ != NULL)
	{
	  fprintf(StockAddrServ,"%s\n",ipstr);
	}
	
      // Envoie d'un premier message avec la taille de la suite
      EnvoieMessage(s, "TailleMessage:%16d", strlen(ipstr));
      // Envoie d'un second message avec le reste
      EnvoieMessage(s, ipstr);
      
	  
      char * proto = RecoieLigne(s) ;
      write(STDOUT_FILENO,proto,strlen(proto));
      fprintf(stdout,"\n");

      //Decryptage du protocole recu => fonction spé?
      char * option = decoupeStr(proto,0,0);
      
      if(strcmp(option,"0") == 0 )
	{
	  //A implementer quand on sort du noeud ' nbrVoisins-- '
	  fprintf(stdout,"option : %s \n",option);
	}
      else if(strcmp(option,"1") == 0 )
	{
	  fprintf(stdout,"%s veut s'inserer dans le noeud !\nVerification de la disponibilite...  \n",ipstr);
	  if(nbrVoisins < NBR_MAX_VOISINS )
	    {
	      nbrVoisins++;
	      fprintf(stdout,"Une place est libre, il y a maintenant %d voisins direct\n",nbrVoisins);
	    }
	  else
	    {
	      fprintf(stdout,"Nombre de voisins maximum\n");
	      // On envoie une reponse contenant les voisins direct du serveur
	    }
	
	}
      else if(strcmp(option,"2") == 0 )
	{	
	  fprintf(stdout,"\nLe client demande un fichier \n");
	  //faire les decoupage petit a petit! ici on passe direct a la lecture du nomFile pour les tests
	  char * tailleAddr = decoupeStr(proto,1,2);
	  int tailleAddrInt = strtol(tailleAddr,(char**)NULL,10);
	  fprintf(stdout,"TailleAddrInt : %d \n",tailleAddrInt);

	  char * ipsrc = decoupeStr(proto,3,3+(tailleAddrInt-1)); //Le calcul semble compliqué et inutile mais est ecrit de cette maniere afin de rendre compte de la fon dont est lu l'ipsrc
	  
	  fprintf(stdout,"ipsrc : %s \n",ipsrc);
	  
	  char * tailleMess2 = decoupeStr(proto,3+tailleAddrInt,3+(tailleAddrInt+1));
	  int tailleMess2Int = strtol(tailleMess2,(char**)NULL,10);
	  fprintf(stdout,"TailleMess2Int : %d \n",tailleMess2Int);
	  
	  char * nomFile = decoupeStr(proto,3+(tailleAddrInt+2),(3+(tailleAddrInt+2)+tailleMess2Int));
	  fprintf(stdout,"nomFile : %s \n",nomFile);

	  fprintf(stdout,"Recherche du fichier en cours . . .\n");
	  /*
	    if (execv("./", "ps") == -1) {
	    perror("execv");
	    return EXIT_FAILURE;
	    }*/
	  
	}
      else if(strcmp(option,"3") == 0 ){
	fprintf(stdout,"Le serveur va se fermer à la demande du Client\n");
	break;
      }
	
      else
	{
	  fprintf(stdout,"Erreur de lecture du protocole\n");
	}
      
      
      
    }
    fclose(StockAddrServ);
    close(s);
  }
  else {
    //On est le client
    char buff[31];
 
    char nomFile[31];
    int choix = 10; // choix > 2 pour etre sur que cela ne quitte pas le client(=0)
    size_t tailleFile ;
    
    AffMenu();
    char message[TAILLE_MAX_MESS];
    do{
      printf("\nQue choisissez-vous ? Tapez 1, 2, 3 ou 0\n");
		
      scanf("%d",&choix); //demande du choix

      switch(choix){
      case 0:
	printf("Quitter\n");
	close(s);
	break;
	
      case 1:
	printf("Vous avez choisi : Demande d'insertion dans le noeux \n\n");
	protocole(message,OPT_INS,0,NULL,"");	
	break;
	
      case 2:
	printf("Vous avez choisi : Demander un fichier \n\n");
	printf("Quel fichier voulez-vous? (30caractère max, evitez les espaces et caractères speciaux\n");
	scanf("%s",nomFile);
	tailleFile=strlen(nomFile);
	protocole(message,OPT_FIC,tailleFile,nomFile,"");
	break;
      case 3:
	printf("Vous avez choisi : Quitter la demo \n\n");
	protocole(message,OPT_CLO,0,NULL,"");
	break;
      default:
	printf("Choix non disponible. Recommencez.\n");
	break;
	
      }
    }while((choix>3));
    
    if(choix != 0)
      {
	int r = recv(s, buff, 30, MSG_WAITALL);
    
	if (r == -1) {
	  perror("recv client");
	}
	
	// J'ajoute le caractère de fin de chaine à la fin du message recu
	buff[r] = '\0';
	fprintf(stdout, "\nLe client a recu '%s'\n", buff);
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
       
	// Envoie du protocole
	tailleEnvoye = EnvoieMessage(s, message);
	printf("tailleEnvoye = %d \n",tailleEnvoye);
      }
    close(s);
    fclose(StockAddrServ);
  }

}


  
