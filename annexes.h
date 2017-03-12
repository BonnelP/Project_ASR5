#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


void protocole(char *message, int option, int taille_mess2, char *mess2,char *ipsrc);

//void LectureProtocole(char *proto,char *ipstr,int* nbrVoisins);

void AffMenu();

char *decoupeStr (const char *src , int ind1 , int ind2);

char *fgets_nonbloquant(char *Saisi, const int taille, FILE *f);

