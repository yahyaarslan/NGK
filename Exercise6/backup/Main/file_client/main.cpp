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

//#define BUFSIZE 1000
#define portno 9000

void write_file(int sockfd){
  int n;
  FILE *fp;
  char *filename;
  char buffer[BUFSIZE];

  fp = fopen(filename, "w");
  while (1) {
    n = recv(sockfd, buffer, BUFSIZE, 0);
    if (n <= 0){
      break;
      return;
    }
    fprintf(fp, "%s", buffer);
    bzero(buffer, BUFSIZE);
  }
  return;
}



int main(int argc, char *argv[])
{
  // TO DO Your own code
  int sockfd,n,new_sock;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  socklen_t addr_size;
  char buffer[BUFSIZE];

  // Check usage
  if (argc != 2)
      error("ERROR: \"Usage: <hostname>.\" \n");

  // Check socket availability
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR: \"While opening socket.\" \n");

  // Check server
  server = gethostbyname(argv[1]);
  if (server == NULL)
    error("ERROR: \"No such host.\" \n");

  // Success
  printf("Starting client on port %d.\n",portno);
  printf("Server specified: %s, port: %d\n",argv[1], portno);


  // Check connectivity
  printf("Trying to connect...\n");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	    error("ERROR: \"Cannot establish connection.\" \n");

  // Send request for file
  printf("Enter name of file to request (directory optional):\n");

  fgets(buffer,sizeof(buffer),stdin);
  n = write(sockfd,buffer,strlen(buffer));  // socket write

  writeTextTCP(n,buffer);

  // Check success in writing filename to server
  if (n < 0)
      error("ERROR: \"While writing to socket.\" \n");

  // Read response
  bzero(buffer,sizeof(buffer));
  n = read(sockfd,buffer,sizeof(buffer));  // socket read

  // Check response
  printf("Size of file %s MB.\n",buffer);
  if (n) // File does not exist
      error("ERROR: \"File does not exist. Exiting...\" \n");

  // Write data
  addr_size = sizeof(serv_addr);
  new_sock = accept(sockfd, (struct sockaddr*)&serv_addr, &addr_size);
  write_file(new_sock);
  printf("[+]Data written in the file successfully.\n");

  // Ending: close socket
  printf("Closing client...\n\n");
  close(sockfd);
  return 0;
}

/**
 * Modtager filstørrelsen og udskriver meddelelsen: "Filen findes ikke" hvis størrelsen = 0
 * ellers
 * Åbnes filen for at skrive de bytes som senere modtages fra serveren (HUSK kun selve filnavn)
 * Modtag og gem filen i blokke af 1000 bytes indtil alle bytes er modtaget.
 * Luk filen, samt input output streams
 *
 * @param fileName Det fulde filnavn incl. evt. stinavn
 * @param sockfd Stream for at skrive til/læse fra serveren
 */
void receiveFile(string fileName, int sockfd)
{
	// TO DO Your own code
}
