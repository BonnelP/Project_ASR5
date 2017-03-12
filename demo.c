#include "socklib.h"
#include "annexes.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>

#define NBR_MAX_VOISINS 3

#define TAILLE_MAX_NOMF 30
#define TAILLE_MAX_IP 15


//taille max du message du protocole = (option) + (taille_addresse)+ ipsrc + taille_mess2 + mess2 +
//                                   :    1     +         2        +   15  +       2      +   45  +
#define TAILLE_MAX_MESS 65

#define OPT_EXI 0 
#define OPT_INS	1
#define OPT_FIC 2
#define OPT_CLO 3 // option pour fermer le client ET le serveur , utile principalement pour les tests
#define REP_INS 4

int main(int argc, char *argv[]) {
  int server = 0;
  int s;
  int tailleEnvoye;
  int nbrVoisins = 0;
  int pid;
  
  //var pour récup addr Client
  char  ipstr[INET6_ADDRSTRLEN];
  int port;

  //fichier de stockage des adresses
  FILE* StockAddrServ = NULL;
  StockAddrServ = fopen("StockAddrServ.txt","a+");
  
  
  //message qui sera envoyer par le protocole
  char message[TAILLE_MAX_MESS];
  
  
  if (StockAddrServ == NULL)
    {
      // On affiche un message d'erreur si on veut
      printf("Impossible d'ouvrir le fichier StockAddr.txt");
      }
  
  if (argc == 2)
    {
      // je suis le serveur (l'argument est le port)
      server = 1;  
    }
  else if (argc == 3)
    {
      // Je suis le client
      server = 0;
    
      // création d'une socket et connexion
      s = CreeSocketClient(argv[1], argv[2]);
    }
  else
    {
      // il y a un probleme car il manque d'argument
      fprintf(stderr, "Usage serveur: %s <port>\n       client: %s <server> <port>\n", argv[0], argv[0]);
      exit(1);
    }


  if (server)
    {
    
      // Création de la socket d'attente
      int sock_attente = CreeSocketServeur(argv[1]);
      if (sock_attente == -1)
	{
	  //fclose(StockAddrServ);
	  exit(1);
	}
      while(1)
	{
	  // attente du client
	  s = AcceptConnexion(sock_attente);
	  if (s < 0){
	    perror("ERREUR acceptCo");
	  }

	  else
	    {
	      pid = fork();
	      if (pid < 0)
		{
		  perror("ERREUR fork pid");
		}
	      if (pid == 0)
		{		  
		  socklen_t len;
		  struct sockaddr_storage addr;

		  //Recuperation @ client
		  len = sizeof addr;
		  getpeername(s, (struct sockaddr*)&addr, &len);
		  // traiter IPv4 et IPv6:
		  if (addr.ss_family == AF_INET)
		    {
		      struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		      port = ntohs(s->sin_port);
		      inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
		    }
		  else
		    { // AF_INET6
		      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		      port = ntohs(s->sin6_port);
		      inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
		    }
		  printf("\nClient IP address: %s\n", ipstr);
		  printf("Client port      : %d\n", port);

		  //On garde une trace des adresses qui ce sont connectees au serveur
		  
		  if (StockAddrServ != NULL)
		    {
		      fprintf(StockAddrServ,"%s %d\n",ipstr,port);
		      fclose(StockAddrServ);
		    }	      
		  
		  char * proto = RecoieLigne(s) ;
		  if (proto != NULL)
		    {
		      write(STDOUT_FILENO,proto,strlen(proto));
		      fprintf(stdout,"\n");

		      //Decryptage du protocole recu cote serveur
		      char * option = decoupeStr(proto,0,0);
      
		      if(strcmp(option,"0") == 0 )
			{
			  fprintf(stdout,"Le client a fermer sa connexion, une place viens de se liberer dans le noeud\n");
			  if (nbrVoisins > 0){nbrVoisins--;}
			}
		  
		      else if(strcmp(option,"1") == 0 )
			{
			  fprintf(stdout,"%s veut s'inserer dans le noeud !\nVerification de la disponibilite...  \n",ipstr);
			  if(nbrVoisins < NBR_MAX_VOISINS )
			    {
			      nbrVoisins++;
			      fprintf(stdout,"Une place est libre, il y a maintenant %d voisins direct\n",nbrVoisins);
			      protocole(message,REP_INS,0,NULL,"");
			      EnvoieMessage(s,message);
			    }
			  else
			    {
			      fprintf(stdout,"Nombre de voisins maximum\n");
			      // On envoie une reponse contenant les voisins direct du serveur
			      protocole(message,REP_INS,1,NULL,"");
			      EnvoieMessage(s,message);
			    }
	
			}
		  
		      else if(strcmp(option,"2") == 0 )
			{	
			  fprintf(stdout,"\nLe client demande un fichier \n");
			  //faire les decoupage petit a petit! ici on passe direct a la lecture du nomFile pour les tests
			  char * tailleAddr = decoupeStr(proto,1,2);
			  int tailleAddrInt = strtol(tailleAddr,(char**)NULL,10);
			  fprintf(stdout,"TailleAddrInt : %d \n",tailleAddrInt);

			  char * ipsrc = decoupeStr(proto,3,3+(tailleAddrInt-1));
			  //Le calcul semble compliqué et inutile mais est ecrit de cette maniere afin de rendre compte de la façon dont est lu l'ipsrc	  
			  if(strcmp(ipsrc,"0.0.0.0") == 0 )
			    {
			      fprintf(stdout,"ipsrc : %s \n",ipstr); 
			    }
			  else
			    {
			      fprintf(stdout,"ipsrc : %s \n",ipsrc);
			    }
		      
			  char * tailleMess2 = decoupeStr(proto,3+tailleAddrInt,3+(tailleAddrInt+1));
			  int tailleMess2Int = strtol(tailleMess2,(char**)NULL,10);
			  fprintf(stdout,"TailleMess2Int : %d \n",tailleMess2Int);
	  
			  char * nomFile;
			  if(tailleMess2Int < 10)
			    {
			      nomFile = decoupeStr(proto,3+(tailleAddrInt+1),(3+(tailleAddrInt+2)+tailleMess2Int));
			    }
			  else
			    {
			      nomFile = decoupeStr(proto,3+(tailleAddrInt+2),(3+(tailleAddrInt+2)+tailleMess2Int));
			    }
			  const char *homedir = getenv("HOME"); //permet de remplacer ~ pour la commande find
			  if (homedir == NULL)
			    {
			      struct passwd *pw = getpwuid(getuid());
			      homedir = pw->pw_dir;
			    }
			  fprintf(stdout,"nomFile : %s \n",nomFile);
			  //fprintf(stdout,"Recherche du fichier en cours . . .\n");
			  EnvoieMessage(s,"Recherche du fichier en cours . . .\n");
			  // system("find ~ -name 'serveur.c' -print");
			  execlp("find","find",homedir,"-name",nomFile,NULL);

			}
		  
		      else if(strcmp(option,"3") == 0 )
			{
			  fprintf(stdout,"Le serveur va se fermer à la demande du Client\n");
			  close(s);
			  exit(0);
			}
	
		      else
			{
			  fprintf(stdout,"Erreur de lecture du protocole\n");
			}
		    }
		}
	      else
		{
		  close(s);
		}
	    }
	}
    }
  
  else if(s != -1)
    { //On est le client 
      char nomFile[31];
      int choix = 10; // choix > 3 pour etre sur que cela ne quitte pas le client(=0)
      int tailleFile ;
    
      AffMenu();
      
      do
	{
	  printf("\nQue choisissez-vous ? Tapez 1, 2, 3 ou 0\n");
	  scanf("%d",&choix); //demande du choix
	  
	  switch(choix)
	    {
	    case 0:
	      printf("Quitter\n");
	      protocole(message,OPT_EXI,0,NULL,"");
	      EnvoieMessage(s, message);
	      close(s);       
	      exit(0);
	
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
	      EnvoieMessage(s, message);
	      close(s);
	      exit(0);
	    default:
	      printf("Choix invalide. Recommencez.\n");
	      break;
	
	    }
	}while((choix > 3));
    
      printf("Le protocole donne : %s \n",message);	
      if(choix != 0)
	{
	  // Envoie du protocole
	  tailleEnvoye = EnvoieMessage(s, message);
	  printf("tailleEnvoye = %d \n",tailleEnvoye);
	}
      /*   
	   char * protoC = RecoieLigne(s) ;
	   if (protoC != NULL)
	   {
	   write(STDOUT_FILENO,protoC,strlen(protoC));
	   fprintf(stdout,"\n");

	   //Decryptage du protocole recu cote client
	   char * optionC = decoupeStr(protoC,0,0);    
	   if(strcmp(optionC,"4") == 0 )
	   {
	   fprintf(stdout,"optionC : %s \n",optionC);
	   char * full = decoupeStr(protoC,1,1);
	   if(strcmp(full,"0") == 0 )
	   {
	   fprintf(stdout,"Le serveur est plein.");
	   }
	   else if(strcmp(full,"1") == 0 )
	   {
	   fprintf(stdout,"L'ajout dans le noeud a bien ete effectue.");
	   }
	   } 
	   }*/
      close(s);
      fclose(StockAddrServ);
    }
}


  
