#include <stdio.h>
#include<ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct config{
	int portno;
	char htmlpath [1000];
};


void readconfig(struct config* C, char* config_file_name)
{
	FILE* fp=fopen(config_file_name,"r");
	int ctr=0;
	char line [1000];
	while (fgets(line, 100, fp) != 0)
	{
		if (line[strlen(line) -1] == '\n')
		{
			line[strlen(line) -1] = 0;
		}

		if(ctr==0)
		{
			int p = atoi(line);
			C->portno =p;
		}
		else if(ctr==1)
		{
			strcpy(C->htmlpath,line);
		}
		ctr++;
    }
}

// if a file is not found, we have to send a 404, file not found status
char* Create404Respone(int* data_size)
{
	printf("FIle not found, sening 404 response");
	char* fnf_header = "HTTP/1.1 404 Not Found\r\n";
	*data_size = strlen(fnf_header);
	return fnf_header;
}

//Read the file from disk, 
//depending on the file extension, create an appropriate file type header,
//make one response with header and data

char* CreateHttpResponse_Binary(char* filename, int file_type,int* data_size)
{
	int MAX_CHUNK = 8*1024*1024;
	*data_size =0;

    FILE *htmlData = fopen(filename, "rb");
	if (!htmlData)
	{
		return Create404Respone(data_size);
	}
	char httpHeader[8000];
    char line[1000];
    char* responseData = (char*)malloc(MAX_CHUNK);
	int maxResposeDataSize = MAX_CHUNK;
	int nResponseBytes =0;
	int bytes_read = 0;

	int iter =0;
    while ((bytes_read =fread(line, 1,1000, htmlData)) != 0) {
		iter++;
		//printf("iter = %d bytes read = %d \n",iter,bytes_read);
		if ((nResponseBytes + bytes_read) > maxResposeDataSize)
		{
			char* newresponseData = (char*)malloc(maxResposeDataSize+MAX_CHUNK);
			memcpy(newresponseData,responseData,nResponseBytes);
			free(responseData);
			responseData = newresponseData;
			maxResposeDataSize += MAX_CHUNK;
		}
        memcpy(responseData+nResponseBytes, line,bytes_read);
		nResponseBytes += bytes_read;
    }
	printf("nresponsebytes = %d\n",nResponseBytes);

	char* final_response =NULL;
	int final_response_len =0;
	
	//find content type from file extension
	//create the response header appropriately
	char* ext = strchr(filename,'.');
	if (!strcmp(ext,".png"))
	{
		sprintf(httpHeader,"HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: %d\r\n\r\n",nResponseBytes);
	}
	else if (!strcmp(ext,".jpg") || !strcmp(ext,".jpeg"))
	{
		sprintf(httpHeader,"HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n",nResponseBytes);
	}
	else{
		sprintf(httpHeader,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n",nResponseBytes);
	}
	
	printf("Http header to send : %s\n",httpHeader);

	final_response_len = strlen(httpHeader)+nResponseBytes;
	final_response = (char*)malloc(final_response_len);
	memcpy(final_response,httpHeader,strlen(httpHeader));
	memcpy(final_response+strlen(httpHeader),responseData,nResponseBytes);
	free(responseData);

	fclose(htmlData);
	*data_size =final_response_len;
	return final_response;

}


//parses the first HTTp request to extract the file name
int GetFileNameToServe(char* http_request, char* file_name)
{
	printf("in get file name\n");
	char command[10];
	int i = 0;
	for(i =0; i< 3; i++)
	{
		command[i]=http_request[i];
	}
	command[i] =0;
	printf("command is %s\n",command);
	// will support only GET command
	if (strcmp(command,"GET"))
	{
		return -1;
	}
	//skip one space after GET
	i++;
	
	//copy the file name from the request
	int k =0;
	while(i <strlen(http_request) && (http_request[i] != ' ') )
	{
		file_name[k] =http_request[i];
		i++;
		k++;
	}
	file_name[k] =0;
	printf("file name requested: %s",file_name);

	return 0;
}

int main(int argc, char* argv [])
{
	struct config C;
	readconfig(&C, argv[1]);

	char str[1000];
	int listen_fd, comm_fd;
	struct sockaddr_in servaddr;
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero( &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(C.portno);
	bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	listen(listen_fd, 10);

	while(1)
	{
		printf("accepting connections ...\n");
		comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
		bzero( str, 1000);
		recv(comm_fd,str,1000,0);
		printf("Echoing back - %s",str);
		//send(comm_fd,str,strlen(str),0);

		char filename[4000];
		strcpy(filename,C.htmlpath);

		char relpath[1000];
		GetFileNameToServe(str,relpath);


		if ( (strlen(relpath) ==0) || (!strcmp(relpath,"/")))
		{
			strcpy(relpath,"index.html");
		}
		printf("relpath is %s\n",relpath);
		strcat(filename,relpath);

		printf("filename is %s\n",filename);
		int resp_len = 0;
		//char* resp = CreateHttpResponse_HTML(filename,0,&resp_len);
		char* resp = CreateHttpResponse_Binary(filename,0,&resp_len);
		
		printf("sending response. Response Length is %d\n",resp_len);
		//printf("%s",resp);
		int chunk =1024;
		int sent_bytes = 0;
		int remaining_bytes = resp_len - sent_bytes;
		while (sent_bytes < resp_len)
		{
			remaining_bytes = resp_len - sent_bytes;
			//send min of chunk or remaining bytes
			int bytes_to_send = (chunk>remaining_bytes)?remaining_bytes:chunk;
			int bs = send(comm_fd,resp+sent_bytes,bytes_to_send,0);
			if (bs != bytes_to_send )
			{
				printf("ERROR - bs = %d bytestosend =%d\n",bs,bytes_to_send);
			}
			sent_bytes += bytes_to_send;
			//printf("sent %d \n",sent_bytes);
		}
		
		printf("closing connection \n");
		close(comm_fd);

	}
}
