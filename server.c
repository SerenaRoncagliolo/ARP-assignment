// SERVER >> G process


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>


// error message
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
// declare the token

struct Token {
	double  data;
	time_t t;
};

// calling main 

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;

	struct Token token;

	char * serverfifo = "/tmp/serverfifo";
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
	 error("ERROR, no port provided\n");
	 exit(1);
	}

	// building the socket.
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
     //   printf("%ld\n",portno);
     //   printf("%ld\n",argv[1]);
     //   printf("%ld\n",argv[5]);
		
	// Connection of the socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
	      sizeof(serv_addr)) < 0) 
	      error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, 
		 (struct sockaddr *) &cli_addr, 
		 &clilen);
	if (newsockfd < 0) 
	  error("ERROR on accept");


	// Opening the server Fifo 
	int fd = open(serverfifo, O_WRONLY); // write-only mode
	if (fd == 0) {
	perror("Cannot open fifo");
	exit(1);
	}

	while(1)
	{
		// Reading the Token from the Socket
		n = read(newsockfd,&token,256);
	     	if (n < 0) error("ERROR reading from socket");
	
		printf("Token Received From Client: %f\n", token.data);

		// Writnig the received Token in the server FIFO
		int nb = write(fd, &token, sizeof(token));
		if (nb == 0){
		perror("Write error\n");
		}
		printf("Token Sent in Server Fifo: %f\n", token.data);
	}
	return 0; 
}
