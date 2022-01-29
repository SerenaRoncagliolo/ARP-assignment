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

int main(int argc, char *argv[])
{
	// declare vars
	int sockfd, newsockfd, portno;
	socklen_t clilen;

	struct Token token;

	char * serverfifo = "/tmp/serverfifo";
	char buffer[256];

	// struct for socket, for handling internet addresses
	/* #include <netinet/in.h>

		struct sockaddr_in {
			short            sin_family;   // e.g. AF_INET
			unsigned short   sin_port;     // e.g. htons(3490)
			struct in_addr   sin_addr;     // see struct in_addr, below
			char             sin_zero[8];  // zero this if you want to
		};
	*/

	struct sockaddr_in serv_addr, cli_addr;

	int n;
	if (argc < 2) {
		// arg given as input
		 error("ERROR, no available port found\n");
		 exit(1);
	}

	// build socket
	// Socket programming is a way of connecting two nodes on a network to 
	// communicate with each other
	// create socket con int sockfd = socket(domain, type, protocol)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr)); // The bzero function can be used to erase the given memory area with zero bytes ( \0 )

	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	// Connection of the socket
	// bind() binds a unique local name to the socket with descriptor socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	      error("ERROR when binding socket");

	listen(sockfd,5);

	clilen = sizeof(cli_addr); // size client
	// accept() call is used by a server to accept a connection request from a client
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	
	if (newsockfd < 0) 
	  error("ERROR when accepting socket");

	// Opening the server FIFO
	int fd = open(serverfifo, O_WRONLY); // write-only mode
	if (fd == 0) 
	{
		perror("FIFO can't be opened");
		exit(1);
	}

	while(1)
	{
		// Reading the Token from the Socket
		n = read(newsockfd,&token,256);
	    if (n < 0) 
			error("ERROR when reading from socket");
	
		printf("The received Token from client is: %f\n", token.data);

		// Write the received Token in the server FIFO
		int nb = write(fd, &token, sizeof(token));
		if (nb == 0)
		{
			perror("Write error\n");
		}
		printf("Token sent: % f\n", token.data);
	}
	return 0; 
}
