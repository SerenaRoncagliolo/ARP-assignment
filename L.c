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
void writeLogReceived(double tokenReceived, double timeReceived, int iter); // Initialize function
void writeLogSent(double tokenSent, double timeSent, int iter); // Initialize function
void error(char *myToken); // Initialize function

// token struct initializations
struct tokenL // Initialize struct
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
    	struct tokenL myTokenReceived;
    	int iter; // counter to keep count of num of iterations 
    	iter = 0; // Set to zero the iterations 
    	close(atoi(argv[2])); // To close writing of the pipe

	while(1)  // Performed on Log
	{
		// read token received
		read(atoi(argv[1]), &myTokenReceived, sizeof(myTokenReceived));  // Read the data from P

		// Flag to check if received correctly
		if(myTokenReceived.received == true)
		{
			writeLogReceived(myTokenReceived.token, myTokenReceived.timestamp, iter);
		}
		else 
		{
        		writeLogSent(myTokenReceived.token, myTokenReceived.timestamp, iter);
        		iter ++;
        	}
    	}

	return(0);
}


// check error token funciton
void error(char *myToken)
{
    perror(myToken);
    exit(0);
}

// function to write on log the received info
void writeLogReceived(double tokenReceived, double timeReceived, int iter)
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
    	fprintf(f, "\n[%d]\n-%f: Token Received: <%f>", iter, timeReceived, tokenReceived );
    
	// close log file
    	fclose(f);
}


// function to write the computed token on the log file
void writeLogSent(double tokenSent, double timeSent, int iter)
{
	FILE * f;
	f = fopen("Log_File.log", "a");
	
	if (f == NULL)
   	{
   	     printf("Cannot open file. \n");
   	     exit(0);
   	}

	fprintf(f, "\n[%d]\n-%f: Token Sent: <%f>",iter, timeSent, tokenSent );   
	fclose(f);
}

