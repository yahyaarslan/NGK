  //============================================================================
  // Description : file_client in C++, Ansi-style
  //============================================================================

  #include <iostream>
  #include <fstream>
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include "iknlib.h"

  using namespace std;
  #define SIZE 1000
  char buffer[BUFSIZE];
  int n = 0;

  void receiveFile(const char *fileName, int socketfd);

  int main(int argc, char *argv[])
  {
    printf("Starting client...\n");

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 4)
    error( "ERROR usage: ""hostname"",  ""port"", ""filename(/path)""");

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL)
    error("ERROR no such host");

    printf("Server at: %s, port: %s\n",argv[1], argv[2]);

    printf("Connect...\n");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    error("ERROR connecting");


    // *** START ***
    // Write filename to server
    sprintf(buffer, "%s", (argv[3]));
    buffer[strlen(buffer)] = 0;
    writeTextTCP(sockfd,buffer);

    // Save filename
    const char *fileName = extractFileName(buffer);
    printf("File name is: %s \n",fileName);

    // Check filesize
    long fileSize = getFileSizeTCP(sockfd);
    printf("File size is: %ld \n",fileSize);

    if (fileSize > 0)
    {
      printf("File Exists. Receiving.\n");
      receiveFile(fileName,sockfd);
    }
    else
      printf("File does not exist. Exiting.\n");

    // Close
    printf("Closing client...\n\n");
    close(sockfd);
    return 0;
  }

  void receiveFile(const char* fileName, int sockfd)
  {
      FILE *fp;
      fp = fopen(fileName, "w");

      do
      {
        n = recv(sockfd, buffer, SIZE, 0);
        printf("Received: %d bytes.\n",n);
        fprintf(fp, "%s", buffer);
        bzero(buffer,SIZE);
      } while (n > 0);

      fclose(fp);
  }
