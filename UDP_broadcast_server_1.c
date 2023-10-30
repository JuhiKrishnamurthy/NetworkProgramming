#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include<arpa/inet.h>

/* Server.c */

int main()
{
   int server_socket;
   struct sockaddr_in server_address, client_address;
   char buf[512];
   unsigned int clientLength;
   int checkCall, message;
   
   /*Create socket */
   server_socket=socket(AF_INET, SOCK_DGRAM, 0);
   if(server_socket == -1)
        perror("Error: socket failed");

   bzero((char*) &server_address, sizeof(server_address));

   /*Fill in server's sockaddr_in*/
   server_address.sin_family=AF_INET;
   server_address.sin_addr.s_addr=htonl(INADDR_ANY);
   server_address.sin_port=htons(22000);

   /*Bind server socket and listen for incoming clients*/
   checkCall = bind(server_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
   if(checkCall == -1)
        perror("Error: bind call failed");

   while(1)
   {
	printf("SERVER: waiting for data from client\n");
	
	clientLength = sizeof(client_address);
	message = recvfrom(server_socket, buf, sizeof(buf), 0,
		  (struct sockaddr*) &client_address, &clientLength);
	if(message == -1)
        perror("Error: recvfrom call failed");

	printf("SERVER: read %d bytes from IP %s(%s)\n", message,
		  inet_ntoa(client_address.sin_addr), buf);

	if(!strcmp(buf,"quit"))
           break;

	strcpy(buf,"ok");

	message = sendto(server_socket, buf, strlen(buf)+1, 0,
		  (struct sockaddr*) &client_address, sizeof(client_address));
	if(message == -1)
        perror("Error: sendto call failed");		

	printf("SERVER: send completed\n");
   }
   checkCall = close(server_socket);
   if(checkCall == -1)
        perror("Error: bind call failed");

}