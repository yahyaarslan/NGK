/* *** Sources: ***
***
Michael Alrøe, Exercise_6_cpp.zip, https://aarhusuniversitet-my.sharepoint.com/:u:/g/personal/au284590_uni_au_dk/EX4PXuND1XxIhZzbFIwcuEQBJxsz5XmFR49ZZqUjZ6vDcw
Last visited: Tuesday, March 15, 2022

Michael Alrøe, Exercise_6_cpp_Filehandling.zip, Exercise_6_cpp_Filehandling.zip, https://aarhusuniversitet-my.sharepoint.com/:u:/g/personal/au284590_uni_au_dk/EdbZ4nw-8P9NglQPkDMORkoB2eFkcHOYK_RwwkeOq-qK1g
Last visited: Tuesday, March 15, 2022

Michael Alrøe, Exercise_6_cpp_SocketDemo.zip, https://aarhusuniversitet-my.sharepoint.com/:u:/g/personal/au284590_uni_au_dk/EdJjECSBIVFGpxzJn3XtRLIBNQxLs6ZYx2oG6oKH1e6fJg
Last visited: Tuesday, March 15, 2022

Linuxhowtos, C_C++_Socket, https://www.linuxhowtos.org/C_C++/socket.htm
Last visited: Tuesday, March 15, 2022

NIKHIL KUMAR TOMAR, File Transfer using TCP Socket in C, https://idiotdeveloper.com/file-transfer-using-tcp-socket-in-c/
Last visited: Tuesday, March 15, 2022
***
*/
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

#define BUFSIZE 1000

using namespace std;

void sendFile(string fileName, long fileSize, int outToClient);
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
    printf("Accept...\n\n");
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //Accepting connection
    if (newsockfd < 0)
    error("ERROR on accept");
    else
    printf("Accepted\n");

    bzero(buffer, 1000); //Cleaner bufferen så den er klar til ny data.

    //nu læses filnavnet fra clienten.
    readTextTCP(buffer, 1000, newsockfd);

    const char *filename = extractFileName(buffer);
    printf("File name is: %s \n",filename);

    long fileSize = check_File_Exists(filename);
    printf("File size is: %ld \n",fileSize);

    // Send fileSize
    char fileSizeBuffer[BUFSIZE];
    sprintf(fileSizeBuffer, "%ld", fileSize);
    writeTextTCP(newsockfd,fileSizeBuffer);

    if (fileSize > 0)
    {
      printf("File exists. Sending file.\n");
      sendFile(filename,fileSize,newsockfd);
    }
    else
    {
      printf("File not found\n");
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
void sendFile(string fileName, long fileSize, int outToClient)
{
  char* buf[1000];

  int n;
  FILE * file = fopen(fileName.c_str(),"rb");

  do
  {
    bzero(buf,1000);
    if (fileSize>=1000)
    {
      fread(buf,1,1000,file);

      n = write(outToClient,buf,1000);
      cout << "Bytes sent: " << n << endl;
      fileSize-=1000;
    }

    else if (fileSize>0)
    {
      fread(buf,1,fileSize,file);

      n = write(outToClient, buf, fileSize);
      cout << "Bytes sent: " << n << endl;
      fileSize = fileSize -n;
    }
  } while(fileSize>0);

  fclose(file);
  return;
}
