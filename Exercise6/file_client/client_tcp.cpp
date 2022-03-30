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

void receiveFile(string fileName, int sockfd, long fileSize);

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
    receiveFile(fileName, sockfd, fileSize);
  }
  else
  printf("File does not exist. Exiting.\n");

  // Close
  printf("Closing client...\n\n");
  close(sockfd);
  return 0;
}

void receiveFile(string fileName, int sockfd, long fileSize)
{
  string placement = extractFileName(fileName.c_str());
  ofstream file(placement, std::ios::binary);

  do
  {
    bzero(buffer,1000);
    if (fileSize>=1000)
    {
      n = read(sockfd,buffer,1000);
      cout << "Bytes recieved: " << n << endl;
      file.write(buffer,1000);
      fileSize = fileSize - 1000;
    }

    else if (fileSize>0)
    {
      n = read(sockfd,buffer,fileSize);
      cout << "Bytes recieved: " << n << endl;
      file.write(buffer,fileSize),
      fileSize = fileSize - n;
    }
  } while(fileSize>0);

}
