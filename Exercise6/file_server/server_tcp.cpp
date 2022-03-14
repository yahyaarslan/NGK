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
#include "iknlib.h"

using namespace std;



void sendFile(const char *fileName, long fileSize, int outToClient);
void send_file(const char *fileName, int sockfd);
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

    printf("Starting server...\n");
    int sockfd, newsockfd, portno = 9000;

    socklen_t clilen;
    char buffer[BUFSIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    //creating socket now.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening socket");

    printf("Binding...\n");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    	 error("ERROR on binding");



    // time for listen.
    printf("Listen...\n");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    for(;;)
    {
        printf("Accept...\n");
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        else
            printf("Accepted\n");

        bzero(buffer, 1000);

        //nu læses filnavnet fra clienten.
        readTextTCP(buffer, 1000, newsockfd);

        const char *filename = extractFileName(buffer);
        long fileSize = check_File_Exists(filename);

        //if(fileSize)

            printf("File exists. Sending file.\n");
          //  writeTextTCP(newsockfd,buffer);
          //  send(sockfd, "%ld",fileSize, sizeof(fileSize), 0);
            send_file(filename, newsockfd);


        //else

        //  printf("File does not exist.\n");
        //  writeTextTCP(newsockfd,buffer);


        printf("Client served.\n");
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
void sendFile(const char *fileName, long fileSize, int outToClient)
{
    // TO DO Your own code

    // open file
    FILE * fp;
    fp = fopen(fileName,"r"); // "r" for read mode of the file.
    int buffer[1000];


    // gentag indtil alt fil data er sendt
    for (int i = 0; i <= fileSize; i + 1000)
    {
        // læs blok fra fil
        ssize_t ret = read(i,fp,1000);
        if (ret < 0)
            error("File does not exist");
        //else // send blok over til client



    }


    // luk fil
}

void send_file(const char *fileName, int sockfd)
{
  FILE *fp;
  int n;
  char data[1000] = {0};

  fp = fopen(fileName, "r");
  if (fp == NULL) {
    perror("[-]Error in reading file.");
    exit(1);
  }

  while(fgets(data, 1000, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      error("Error in sending file.");
    }
    bzero(data, 1000);
  }
  fclose(fp);
  return;
}
