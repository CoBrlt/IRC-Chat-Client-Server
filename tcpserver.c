#include<stdio.h>	
#include<string.h> 
#include<stdlib.h> 
#include <unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define SERVER "127.0.0.1"  //localhost
#define BUFLEN 512	    //Buffer length
#define PORT   1234	    //Destination port

void stop(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
	struct sockaddr_in servaddr, cli_addr;
	int sockfd, newsockfd, clilen;
	char message[BUFLEN+1];

	
	if ( (sockfd=socket(AF_INET, SOCK_STREAM , 0)) == -1)
	{
		stop("socket");
	}

	memset((char *) &servaddr, 0, sizeof(servaddr));
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	
	if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
	{
		stop("bind failed. Error");
	}

	if (listen(sockfd,5) != 0)
        {
                stop("listen failed. Error");
        }

	memset(&cli_addr, 0, sizeof(cli_addr));
	clilen = sizeof(cli_addr);
	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen)) < 0){
		stop("accept failed. Error");
	}	

	for(int i=0;i<1000;i++){
		bzero(&message,BUFLEN+1);
		if( recv(newsockfd, message, BUFLEN, 0) < 0 ){
			stop("recv failed. Error");
		}
		printf("Recv : %s\n", message);	

		if (send(newsockfd, message, strlen(message) , 0)==-1)
		{
			stop("send()");
		}
	}
	close(newsockfd);
	close(sockfd);
	return 0;
}
