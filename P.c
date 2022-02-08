#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <time.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define SIZE 256


// **** PROCESS P ****
// process that compute new token, following sine wave

// function declarations
double computeNewToken(double received_token, double time_delay, double RF ); 
void error(char *myToken); 
void writeLog(char string[50]); 

// token struct
struct myToken
{
    double token;
    double timestamp;
};

// log token struct
struct tokenL 
{
    double token;
    double timestamp;
    bool received;
};


// Main
int main(int argc, char *argv[])
{
    

	printf("PROCESS P: start execution\n");
	// file descriptor
    	fd_set file_descriptor_select;
    
	// Create Variables
    	int  processLReceivedInt, my_select, max_fd, res, res1, res2, res3, res4;
	
	// first pipe	
	int pipe1 = atoi(argv[5]); 
	// second pipe	
	int pipe2 = atoi(argv[7]); 

	// token variables
	double tokenReceived, tokenToBeSent,timestampReceived, timestampSent, dt, rf, waitTime;
	
	// boolean checks
	bool cont_while, unconnected;

	// char to save signal
    	char processLReceived[SIZE];

	// token structs
	struct myToken receivedMessage;
	struct myToken messageToSend;
	struct tokenL L_Sent, L_Received;

	// atof() convert string to double
	// set frequency
    	rf = atof(argv[1]);
	// set waiting time
	waitTime = atof(argv[2])/1000000; // Set a Waiting Time

	processLReceivedInt= 1;

	tokenReceived = 0.0;
	tokenToBeSent = 0.0;

	timestampReceived = time(NULL);


   	printf("RF=%f   WT=%f\n", rf, waitTime);

    	// vars for sockets
	int sockfd, portno, n; 
    	struct sockaddr_in serv_addr; 	

	// store port number
	portno = atoi(argv[4]); 
	
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
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
		error((char*)"PROCESS P: error while opening socket\n");

	}
	
	//Socket variables initialization
	bzero((char *) &serv_addr, sizeof(serv_addr));

	// in this version of the code, the server should be on the same machine
	// done because could not be tested on multiple machines
    	if(inet_pton(AF_INET, argv[3], &serv_addr.sin_addr)<=0)
    	{
	
        	error((char*)"PROCESS P: no adress found, please insert a valid address\n");
	}
	// struct
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);


	// Server Connection
	// The connect() system call connects the socket referred to by the
	// file descriptor sockfd to the address specified by addr.
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
        	error((char*)"ERROR during connection\n");
	}

	// While connection is estabilished
    	do
	{
	        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		{
			// nothing
        	}
	        else
		{
			unconnected = false;
            		printf("PROCESS P: connection estabilished\n");
        	}
		if (!unconnected)
        	{
            		cont_while = false;
        	}
	}
	while (cont_while);


    	while(1) // Read from SELECT
    	{
		// Initializes the file descriptor set fdset to have zero bits 
		// for all file descriptors.
		FD_ZERO(&file_descriptor_select); 
		// Sets the bit for the file descriptor fd in the file descriptor set fdset.
		FD_SET(pipe1, &file_descriptor_select); 
        	FD_SET(pipe2, &file_descriptor_select); 

        	max_fd = pipe1 >pipe2 ? pipe1 : pipe2;

		// select() allows a program to monitor multiple file descriptors,
		// waiting until one or more of the file descriptors become "ready"
		// for some class of I/O operation (e.g., input possible). 
       
        	my_select = select(max_fd+1, &file_descriptor_select, NULL, NULL, NULL);

        	if (my_select == -1)
        	{
           	 	error((char*)"Error with select()\n");
        	}

        	// Three cases
        	if(FD_ISSET(pipe1, &file_descriptor_select) && (FD_ISSET(pipe2, &file_descriptor_select) && (processLReceivedInt == 1)))
		{
           
			// Read from process S, given by first pipe
			res1 = read(pipe1, &processLReceived, sizeof(processLReceived));

            		//from char to int
            		sscanf(processLReceived, "%d", &processLReceivedInt);
            
			// Read from process G, given by first pipe
            		
            		res2 = read(pipe2, &receivedMessage, sizeof(receivedMessage));

            		tokenReceived = receivedMessage.token;

            		timestampReceived = receivedMessage.timestamp;
		}
		else if (FD_ISSET(pipe1, &file_descriptor_select))
		{
		    
		    // Read from S
		    res = read(pipe1, &processLReceived, sizeof(processLReceived));

		    //Convert from char to int
		    sscanf(processLReceived, "%d", &processLReceivedInt);
		}
	        else if (FD_ISSET(pipe2, &file_descriptor_select) && (processLReceivedInt == 1))
	        {
			// Read from G
			res = read(pipe2, &receivedMessage, sizeof(receivedMessage));

			//Convert from char to double
			tokenReceived = receivedMessage.token;
			timestampReceived = receivedMessage.timestamp;
		}


		// Receive-Send Tokens
		if(processLReceivedInt == 1) 
	        {
			//Send the received token to L to write it on log file
			L_Received.token = tokenReceived;
			L_Received.timestamp = timestampReceived;
			L_Received.received = true;
			write(atoi(argv[6]), &L_Received, sizeof(L_Received));

            		//Compute the sending token
            		timestampSent = time(NULL); //timestamp
	
			dt =  waitTime + timestampSent - timestampReceived; //Difference time

			printf("\nTime Received: %f.\nTime Sent: %f\nDT: %f\n", timestampReceived, timestampSent, dt);

		    	//Creating the message to send to G using the socket
		    	tokenToBeSent = computeNewToken(tokenReceived, dt, rf);

		    	messageToSend.token = tokenToBeSent;
			messageToSend.timestamp = timestampSent;

            		printf("\nReceiving/Sending tokens.\nP: Token Received: %f.\nP: Token Sent: %f\n", tokenReceived, tokenToBeSent);

            		n = write(sockfd, &messageToSend, sizeof(messageToSend)); //Write message on socket
		    	if (n < 0)
			{
			        error((char*)"\nERROR while writing on socket.\n");
			}

			//Send the sent token to L for Log file
		    	L_Sent.token = tokenToBeSent;
			L_Sent.timestamp = waitTime + timestampSent; 
			L_Sent.received = false;
			write(atoi(argv[6]), &L_Sent, sizeof(L_Sent));
        	}
		else if(processLReceivedInt == 0)    //If signal received from S is =0, start receiving and sending tokens
		{

	        	writeLog((char*)"PROCESS P: stop communication of tokens.\n");
	
	        }
		else
		{
			printf("\nERROR.\n");
		}

        	//Wait for 2 seconds 
        	sleep(2);

	} // end while loop

	return 0;

}


// Functions
double computeNewToken(double received_token, double time_delay, double RF )
{
	// new token formula computation
    	double computeNewToken;
    	computeNewToken = received_token + time_delay*(1.0 - (pow(received_token,2.0))/2)*2*M_PI*RF;
    	return computeNewToken;
}


void error(char *myToken)
{
	perror(myToken);
	exit(0);
}


void writeLog(char string[50])
{
    	FILE * f;
	time_t t = time(NULL);

    	struct tm * tm = localtime(&t);

    	f = fopen("Log_File.log", "a+");

	if (f == NULL)
    	{
        	printf("PROCESS P: can't open file\n");
        	exit(0);
	}

	fprintf(f, "\n-%s P: %s\n", asctime(tm), string);
	fclose(f);

}
