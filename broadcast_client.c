#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
#include<arpa/inet.h>
int main(int argc,char **argv)
{
	int sockfd,n;
	//char sendline[100];
	char recvline[100];
	struct sockaddr_in servaddr;
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	bzero(&servaddr,sizeof servaddr);
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(22000);
	servaddr.sin_addr.s_addr = htonl("192.16.95.255");

	//inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));
	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	//connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	while(1)
	{
		//bzero( sendline, 100);
		bzero( recvline, 100);
		//fgets(sendline,100,stdin); /*stdin = 0 , for standard input */

		//send(sockfd,sendline,strlen(sendline),0);
		int b = 1;
		//setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,b,sizeof(b));
		recvfrom(sockfd,recvline,100,0, (struct sockaddr*)&servaddr,sizeof(servaddr));
		printf("%s",recvline);
	}
}