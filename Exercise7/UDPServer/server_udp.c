/* Creates a datagram server.  The port
   number is passed as an argument.  This
   server runs forever */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

int sock, length, n;
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in from;
char buf[1024];

// Read file
FILE * fp;

void getUL();
void sendU();
void sendL();
void sendElse();

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
   sock=socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("Opening socket");
   length = sizeof(server);
   bzero(&server,length);
   server.sin_family=AF_INET;
   server.sin_addr.s_addr=INADDR_ANY;
   server.sin_port=htons(9000);
   if (bind(sock,(struct sockaddr *)&server,length)<0)
       error("binding");
   fromlen = sizeof(struct sockaddr_in);

   while (1)
   {
     getUL();
   }

   return 0;
 }

 void getUL()
 {
   n = recvfrom(sock,buf,1024,0,(struct sockaddr *)&from,&fromlen);
   if (n < 0) error("recvfrom");
   write(1,"Received a datagram: ",21);
   write(1,buf,n);

   if ((buf[0] == 'U') || (buf[0] == 'u'))
   {
     //n = sendto(sock,"Got command 'U/u' \n",30, 0,(struct sockaddr *)&from,fromlen);
     //if (n  < 0) error("sendto");
     sendU();
   }

   else if ((buf[0] == 'L') || (buf[0] == 'l'))
   {
     //n = sendto(sock,"Got command 'L/l' \n",30, 0,(struct sockaddr *)&from,fromlen);
     //if (n  < 0) error("sendto");
     sendL();
   }

   else
   {
     sendElse();
   }
 }

 void sendU()
 {
   //*** Read file***
   fp = fopen("/proc/uptime", "rb");   // read binary
   n = fread(buf, 1, sizeof(buf), fp);  // Automatic seek!
   fclose(fp);

   // print file
   for (int i=0; i<n; i++) printf("%i ", buf[i]);
   printf("\n");

   // send file
   n = sendto(sock,buf,30, 0,(struct sockaddr *)&from,fromlen);
   if (n  < 0) error("sendto");
 }

void sendL()
{
  //*** Read file***
  fp = fopen("/proc/loadavg", "rb");   // read binary
  n = fread(buf, 1, sizeof(buf), fp);  // Automatic seek!
  fclose(fp);

   // print file
  for (int i=0; i<n; i++) printf("%i ", buf[i]);
  printf("\n");

  // send file
  n = sendto(sock,buf,30, 0,(struct sockaddr *)&from,fromlen);
  if (n  < 0) error("sendto");
}

void sendElse()
{
  n = sendto(sock,"Unknown command.",40, 0,(struct sockaddr *)&from,fromlen);
  if (n  < 0) error("sendto");
}
