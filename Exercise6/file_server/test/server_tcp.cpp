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
        readTextTCP(buffer, sizeof(buffer),newsockfd);
        printf("checkpoint 1\n");


        const char *filename = extractFileName(buffer);
        printf("checkpoint 2\n");
        long fileSize = check_File_Exists(filename);
        printf("checkpoint 3\n");

        //writeTextTCP(newsockfd,buffer);
        printf("checkpoint 4\n");
        if(fileSize)
        {
            printf("checkpoint 4,5\n");
            sendFile(filename, fileSize, newsockfd);
        }
        else
        {
            error("File not found\n");          
        }
        printf("checkpoint 4.75\n");
        close(newsockfd);
        printf("checkpoint 4.76\n");
    }
    close(sockfd);
    printf("checkpoint 4.99\n");
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
    printf("checkpoint 5\n");
    // open file
    FILE * fp;
    fp = fopen(fileName,"wb"); // "r" for read mode of the file.
    int buffer[1000];
    printf("checkpoint 6\n");

    // gentag indtil alt fil data er sendt
    for (int i = 0; i <= fileSize; i + 1000)
    {
        // læs blok fra fil
        ssize_t textFromFile = read(*fileName, buffer, sizeof(buffer));
        printf("checkpoint 7\n");
        if (textFromFile < 0)
            error("File does not exist");

        else // send blok over til client
        {
            ssize_t sentData = write(outToClient,buffer,sizeof(buffer));
            printf("checkpoint 8\n");
            if (sentData)
                printf("1000 data sent from %d", i);
            else
                error("Data not transmitted");
        }
        bzero(buffer, sizeof(buffer));
    }


    // luk fil
    fclose(fp);
    return;
}

/*void send_file(FILE *fp, int sockfd){
  int n;
  char data[1000] = {0};

  while(fgets(data, SIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      error("Error in sending file.");
    }
    bzero(data, SIZE);
  }
  fclose(fp);
  return;
}*/
