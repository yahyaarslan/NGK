//============================================================================
// Description : file_server in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include "iknlib.c"
#include "iknlib.h"
//#include "myHeader.h"
using namespace std;

#define portno 9000

//void sendFile(string fileName, long fileSize, int outToClient);

void send_file(FILE *fp, int sockfd){
  int n;
  char data[BUFSIZE] = {0};

  while(fgets(data, BUFSIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, BUFSIZE);
  }
}

void error(const char *msg)
{
	//perror(msg); // prints Success?
  printf(msg,1);
	exit(1);
}

/**
 * main starter serveren og venter på en forbindelse fra en klient
 * Læser filnavn som kommer fra klienten.
 * Undersøger om filens findes på serveren.
 * Sender filstørrelsen tilbage til klienten (0 = Filens findes ikke)
 * Hvis filen findes sendes den nu til klienten
 *
 * HUSK at lukke forbindelsen til klienten og filen når denne er sendt til klienten
 *
 * @throws IOException
 *
 */
int main(int argc, char *argv[])
{
    // TO DO Your own code
    int sockfd, newsockfd,n;
    FILE *fp;

    socklen_t clilen;
    char buffer[BUFSIZE];
    char bufferTx[BUFSIZE];
    struct sockaddr_in serv_addr, cli_addr;

    // Check socket availability
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      error("ERROR: \"While opening socket.\" \n");

    // Check binding
    printf("Binding...\n");
  	bzero((char *) &serv_addr, sizeof(serv_addr));
  	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_addr.s_addr = INADDR_ANY;
  	serv_addr.sin_port = htons(portno);
  	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  		error("ERROR: \"While binding.\" \n");

    printf("Server started.\n");
    listen(sockfd,5);

    clilen = sizeof(cli_addr);

    for(;;)
    {
      // Wait for incoming accept request
      printf("Waiting to accept incoming request...\n");
  		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

      // Check after incoming request
      if (newsockfd < 0)
        error("ERROR: \"While accepting.\" \n");
      else
        printf("Accepted\n");

      // Read socket and get requested filename (account for dir.)
      bzero(buffer,sizeof(buffer));
      n = read(newsockfd,buffer,sizeof(buffer));

      // Check reading socket
      if (n < 0)
        error("ERROR: \"While reading from socket.\" \n");
      printf("Requested file: %s\n",buffer);

      // Get filesize of requested file
      //if (!check_File_Exists(extractFileName(buffer)))
        //printf("File does not exist dummy");
    //  snprintf(bufferTx, sizeof(bufferTx), "%ld",GetFileSize(buffer));

    fp = fopen(buffer, "r");
    if (fp == NULL) {
      perror("[-]Error in reading file.");
      exit(1);
    }

    send_file(fp, sockfd);
    printf("[+]File data sent successfully.\n");
      // Write filesize
      n = write(newsockfd,bufferTx,strlen(bufferTx));
      if (n < 0)
        error("ERROR: \"While writing to socket.\" \n");

      close(newsockfd);
    }

    close(sockfd);
    return 0;
}

/**
 * Sender filen som har navnet fileName til klienten
 *
 * @param fileName Filnavn som skal sendes til klienten
 * @param fileSize Størrelsen på filen, 0 hvis den ikke findes
 * @param outToClient Stream som der skrives til socket
     */
