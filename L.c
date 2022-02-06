#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
#include <stdbool.h>



#define SIZE 256


// **** PROCESS L ****
// Process L writes data events in a  log file 

// functions declarations
void WritingLogReceived(double ReceivedToken, double ReceivedTime, int Iteration); // Initialize function
void WritingLogSent(double SentToken, double TimeSent, int Iteration); // Initialize function
void error(char *my_token); // Initialize function

// token struct initializations
struct my_token_to_L // Initialize struct
{
	double token;
	double timestamp;
	bool received; // this time we need boolean
};

// Main
int main(int argc, char *argv[])
{


	printf("PROCESS L: start execution\n");
	// init token structs
    	struct my_token_to_L my_token_received;
    	int Iteration; // counter to keep count of num of iterations 
    	Iteration = 0; // Set to zero the iterations 
    	close(atoi(argv[2])); // To close writing of the pipe

	while(1)  // Performed on Log
	{
		// read token received
		read(atoi(argv[1]), &my_token_received, sizeof(my_token_received));  // Read the data from P

		// Flag to check if received correctly
		if(my_token_received.received == true)
		{
			WritingLogReceived(my_token_received.token, my_token_received.timestamp, Iteration);
		}
		else 
		{
        		WritingLogSent(my_token_received.token, my_token_received.timestamp, Iteration);
        		Iteration ++;
        	}
    	}

	return(0);
}


// check error token funciton
void error(char *my_token)
{
    perror(my_token);
    exit(0);
}

// function to write on log the received info
void WritingLogReceived(double ReceivedToken, double ReceivedTime, int Iteration)
{
	// open file to write on
    	FILE * f;
    	f = fopen("Log_File.log", "a");
	
	// if error
    	if(f == NULL)
    	{
        	printf("PROCESS L: Cannot open file. \n");
        	exit(0);
    	}

	// print token received
    	fprintf(f, "\n[%d]\n-%f: Token Received: <%f>", Iteration, ReceivedTime, ReceivedToken );
    
	// close log file
    	fclose(f);
}


// function to write the computed token on the log file
void WritingLogSent(double SentToken, double TimeSent, int Iteration)
{
	FILE * f;
	f = fopen("Log_File.log", "a");
	
	if (f == NULL)
   	{
   	     printf("Cannot open file. \n");
   	     exit(0);
   	}

	fprintf(f, "\n[%d]\n-%f: Token Sent: <%f>",Iteration, TimeSent, SentToken );   
	fclose(f);
}

