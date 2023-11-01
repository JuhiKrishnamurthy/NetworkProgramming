#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
int main()
{
	char str[100];
	int sock_id;
	struct sockaddr_in servaddr , cliaddr;
	sock_id = socket(AF_INET, SOCK_DGRAM, 0);
	bzero( &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl("192.168.1.255");
	servaddr.sin_port = htons(22000);
	//bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	//listen(sock_id, 10);

	while(1)
	{
		//comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
		bzero( str, 100);
		//recvfrom(comm_fd,str,100,0,);
		//printf("Echoing back - %s",str);
		int b = 1;
		fgets(str,100,stdin);
		setsockopt(sock_id,SOL_SOCKET,SO_BROADCAST,(const void*)&b,sizeof(b));
		sendto(sock_id,str,strlen(str),0,(struct sockaddr*)&cliaddr,sizeof(cliaddr));
		pclose(sock_id);
	}
}