#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>

// PROCESS G
// it received the token and dispatch it to process P

#define SIZE 256

// define struct of the token the user should insert
struct my_token   // Define struct
{
    double token;
    double timestamp;
};

void error(char *my_token) // Define function
{
	printf("%s\n",my_token);
	exit(0);
}


// Main
int main(int argc, char *argv[])
{	
	// token variables
    	char bufferT[SIZE]; 
    	double tokenSent;
	double tokenReceived; 
    	tokenSent = 0; // init
	
	// struct for token
    	struct my_token messSent, messReceived; // init structs
	
	// print
    	printf("PROCESS G: Start execution\n");

	// init vars
    	int sockfd;
	int portno;
 	int newsockfd;
	int clilen;
	int n; 

    	// Internet adresses
	// Structures for handling internet addresses
	/*
		struct sockaddr_in {
		    short            sin_family;   // e.g. AF_INET
		    unsigned short   sin_port;     // e.g. htons(3490)
		    struct in_addr   sin_addr;     // see struct in_addr, below
		    char             sin_zero[8];  // zero this if you want to
		};
	*/
	struct sockaddr_in serv_addr, cli_addr; 

	// hostent structure is used by functions to store information about a given host
	struct hostent *server;

	// store port number
    	portno = atoi(argv[1]);  

    	printf("PROCESS G: Port Number = %d\n", portno);
	
	// *** SOCKET ***
	/*
		Socket programming is a way of connecting two nodes on a 
		network to communicate with each other. One socket(node) 
		listens on a particular port at an IP, while other socket 
		reaches out to the other to form a connection. Server forms 
		the listener socket while client reaches out to the server.
	*/
	//int sockfd = socket(domain, type, protocol)
	// sockfd socket descriptor 
	// domain -> specifies communication domain
	// type -> communication type
	// protocol -> protocol value for IP
    	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Socket Creation
	
	// check if created correctly
    	if (sockfd < 0)
    	{
        	error((char*)"PROCESS G: error while opening socket\n");
	}

	//Initialize socket
    	bzero((char *) &serv_addr, sizeof(serv_addr));
	// struct
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

    	// Link socket to address
	// int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
 	/* When a socket is created with socket(), it exists in a name
       	   space (address family) but has no address assigned to it.  
	   bind() assigns the address specified to the socket
	*/
    	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    	{
         	error((char*)"PROCESS C: error when binding socket\n");
    	}

	// The listen() call indicates a readiness to accept client connection
	// requests. It transforms an active socket into a passive socket.
	// Once called, socket can never be used as an active socket to initiate connection requests
    	listen(sockfd, 5);

    	clilen = sizeof(cli_addr);
	// The accept() call is used by a server to accept a connection 
	// request from a client. When a connection is available the socket 
	// created is ready for use to read data from the process that 
	// requested the connection
    	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,  (socklen_t*)&clilen);
    
    	if (newsockfd < 0)
	{    
		error((char*)"PROCESS G: error while accepting the client\n");
	}

	while(1)
	{
        	// read()
		// From the file indicated by the file descriptor fs, 
		// the read() function reads N bytes of input into the memory 
		// area indicated by buf. A successful read() updates the access
		// time for the file.
        	n = read(newsockfd, &messReceived, sizeof(messReceived)); // Read the socket

        	if (n < 0)
            		error((char*)"ERROR while reading from socket\n");

        	printf("PROCESS G: Token value:%f, Timestamp: %f\n", messReceived.token, messReceived.timestamp);

       		// create new token
        	tokenSent = messReceived.token; 
		
		// The function int atoi(const char *str) converts the string 
		// argument str to an integer (type int).
        	close(atoi(argv[2]));

        	// update struct
        	messSent.token = tokenSent; // Create message
        	messSent.timestamp = time(NULL);

        	// write new token
		// write() function writes N bytes from buf to the file or socket 
		// associated with fs. N should not be greater than INT_MAX 
		// (defined in the limits.h header file). If N is zero, write() 
		// simply returns 0 without attempting any other action.
        	write(atoi(argv[3]), &messSent, sizeof(messSent));
	}

}
