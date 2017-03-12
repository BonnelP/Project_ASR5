#include "annexes.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define NBR_MAX_VOISINS 3


void protocole(char *message, int option, int taille_mess2, char *mess2,char *ipsrc)
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
      strcat(message,"0.0.0.0");
      //s'il n'y a pas d'ipsrc on met 0.0.0.0 (qui ne peut pas etre une ip src)
    }   
   
    char taille[2]; //pour stocker au plus 2 digit : la taille du nom du fichier
    sprintf(taille,"%d",taille_mess2);
    strcat(message, taille);     
    strcat(message, mess2);
    break;
    
  case 3: ; //avoir un ligne vide declarer permet de contrer l'erreur gcc "a label can only be part of a statement and a declaration is not a statement"
    // Si on repond a la demande d'insertion de le noeud, le protocole sera comme ceci:
    // type : #|#(full)|##|mess2
    // full=0 : le noeud possède au moins une place; =1 : le noeud est plein.
    //on utilisera la var. taille_mess2 pour avoir la valeur du full.
    // mess2 contiendra quoiqu'il arrive la liste des voisins direct
    char full[1];
    sprintf(full,"%d",taille_mess2);
    strcat(message,full);

    break;

  }

}

/*
void LectureProtocole(char *proto,char *ipstr,int* nbrVoisins)
{

  write(STDOUT_FILENO,proto,strlen(proto));
  fprintf(stdout,"\n");

  //Decryptage du protocole recu => fonction spé?
  char * option = decoupeStr(proto,0,0);
      
  if(strcmp(option,"0") == 0 )
    {
      fprintf(stdout,"Le client a fermer sa connexion, une place viens de se liberer dans le noeud");
      if (nbrVoisins > 0){nbrVoisins--;}
      fprintf(stdout,"option : %s \n",option);
    }
  else if(strcmp(option,"1") == 0 )
    {
      fprintf(stdout,"%s veut s'inserer dans le noeud !\nVerification de la disponibilite...  \n",ipstr);
      if(nbrVoisins < NBR_MAX_VOISINS )
	{
	  nbrVoisins++;
	  fprintf(stdout,"Une place est libre, il y a maintenant %d voisins direct\n",nbrVoisins);
	  //protocole(message,REP_INS,);
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

      char * ipsrc = decoupeStr(proto,3,3+(tailleAddrInt-1));
      //Le calcul semble compliqué et inutile mais est ecrit de cette maniere afin de rendre compte de la façon dont est lu l'ipsrc
	  
      fprintf(stdout,"ipsrc : %s \n",ipsrc);
	  
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
	  nomFile = decoupeStr(proto,3+(tailleAddrInt+1),(3+(tailleAddrInt+2)+tailleMess2Int));
	}
      fprintf(stdout,"nomFile : %s \n",nomFile);

      fprintf(stdout,"Recherche du fichier en cours . . .\n");

	  
    }
  else
    {
      fprintf(stdout,"Erreur de lecture du protocole\n");
    }

}
*/

void AffMenu()
{
  printf("\nMenu\n");
  printf("1 : Demande d'insertion dans le noeux\n");
  printf("2 : Demander un fichier\n");
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

char *fgets_nonbloquant(char *Saisi, const int taille, FILE *f) {
  int oldattr = fcntl(fileno(f), F_GETFL);
  if (oldattr==-1)
    {
      perror("fcntl");
      exit(1);
    }
  // on ajoute l'option non blocante
  int r = fcntl(fileno(f), F_SETFL, oldattr | O_NONBLOCK);
  if (r==-1)
    {
      perror("fcntl");
      exit(1);
    }

  char * res = fgets(Saisi,taille,stdin);


  r = fcntl(fileno(f), F_SETFL, oldattr);
  if (r==-1)
    {
      perror("fcntl");
      exit(1);
    }

  return res;
}

