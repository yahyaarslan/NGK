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

    printf("Binding...\n"); //Binding
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
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //Accepting connection
        if (newsockfd < 0)
            error("ERROR on accept");
        else
            printf("Accepted\n");

        bzero(buffer, 1000); //Cleaner bufferen så den er klar til ny data. 

        //nu læses filnavnet fra clienten.
        readTextTCP(buffer, 1000, newsockfd);

        const char *filename = extractFileName(buffer);
        long fileSize = check_File_Exists(filename);
        
        if (fileSize > 0)
        {
        printf("File exists. Sending file.\n");
        send_file(filename, newsockfd);
        }
        else
        {
          error("File not found");
        }
        printf("Client served.\n");
        close(newsockfd);
    }

    close(sockfd); //Close the connection. 
    return 0;
}

/**
 * Sender filen som har navnet fileName til klienten
 *
 * @param fileName Filnavn som skal sendes til klienten
 * @param sockfd Stream som der skrives til socket
     */


void send_file(const char *fileName, int sockfd)
{
  //Open file
  FILE *fp;
  int n;
  char data[1000] = {0};

  fp = fopen(fileName, "r");
  if (fp == NULL) {
    error("[-]Error in reading file.");
  }
    //Keep sending data until done
  while(fgets(data, 1000, fp) != NULL) 
  {
    if (send(sockfd, data, sizeof(data), 0) == -1) 
    {
      error("Error in sending file.");
    }
    bzero(data, 1000);
  }
  fclose(fp); //Close the file
  return;
}
