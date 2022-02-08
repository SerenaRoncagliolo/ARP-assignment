#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <sys/select.h>

#define SIZE 256

// **** PROCESS S ****
// it receives console messages from terminal as posix signals
// the signal will be sent to process P

// flags to handle the received signal
int flag1, flag2, flagLog;

// functions declaration
void printLog();
void writeLog(char string[50]);

// Handling SIGUSR1 signal 
void Handler_SIGUSR1(int signo)

{
    flag1 = 1;

}

// Handling SIGUSR2 signal 
void Handler_SIGUSR2(int signo)

{

    flag2 = 1;

}

// Handling SIGINT signal 
void Handler_SIGINT(int signo)
{

    flagLog = 1;

}

// Main
int main(int argc, char *argv[])
{
    
	printf("PROCESS S: start execution \n");


	//Pipe from S to P
    	int pipe_sp[2];
    	pipe_sp[0] = atoi(argv[1]);
    	pipe_sp[1] = atoi(argv[2]);

    	//Print on Log file essential information
    	writeLog((char*)"PROCESS S: Input Selection\n");
    	printf("\n There are 3 possible inputs:\n");
    	printf("\n -SIGUSR1: Start Receiving Tokens \n-IGUSR2: Stop Receiving Tokens \n-SIGINT: Output Log File\n");
    	printf("\nTo run the S process, open another termina and write \nkillall  (SIGUSR1 / SIGUSR2 / SIGINT) S\n");
    	printf("\nEnter SIGUSR1, SIGUSR2 or SIGINT:\n");

    	//Check inputs
    	if (signal(SIGUSR1, Handler_SIGUSR1) == SIG_ERR)
    	{
        	printf("\nPROCESS S: ERROR can't execute SIGUSR1\n");
    	}

    	if(signal(SIGUSR2, Handler_SIGUSR2) == SIG_ERR)
    	{
        	printf("\nPROCESS S: ERROR can't execute SIGUSR2\n");
	}
	if (signal(SIGINT, Handler_SIGINT) == SIG_ERR)
    	{
        	printf("\nPROCESS S: ERROR can't execute SIGINT\n");
        	int selection = 0;
	}
	
	char selectSignal[SIZE];
	
    	while(1)  //While receiving inputs
    	{
        	if (flag1 == 1 )
        	{

			printf("\nPROCESS S: SIGUSR1 signal received, send tokens)\n"); 

			writeLog((char*)"Start Receiving and Sending Tokens\n");
			// If input == 1 1, P reads and writes tokens
		    	strcpy(selectSignal, "1"); 
		    	close(pipe_sp[0]); 
		    	write(pipe_sp[1], &selectSignal, sizeof(selectSignal));
		}

		if (flag2 == 1 )
		{
            		printf("Stop Sending Tokens (SIGUSR2 received)\n");

            		writeLog((char*)"Stop Receiving Tokens\n");
			// If input == 0, P stops to read and send tokens
            		strcpy(selectSignal, "0"); 
            		close(pipe_sp[0]);
            		write(pipe_sp[1], &selectSignal, sizeof(selectSignal));
		}

		if (flagLog == 1 )
        	{
			printf("Printing Log: (received SIGINT)\n");
			writeLog((char*)"Print Log\n");
			printLog(); // Writes on Log File

        	}

		//Set flags to 0
		flag1 = 0;
		flag2 = 0;
		flagLog = 0;


        	memset(selectSignal, 0, SIZE);


        	//Wait for 2 seconds to have time to perform everything well
        	sleep(2);

    	}
}


// functions to write on the Log File
void writeLog(char string[50])
{

    	FILE * f;
	time_t t = time(NULL);

	struct tm * tm = localtime(&t);

    	f = fopen("Log_File.log", "a+");

	if(f == NULL)
    	{
        	printf("ERROR: Can't open file\n");
        	exit(0);
    	}

    	fprintf(f, "\n-%s\nS: %s\n", asctime(tm), string );
    	fclose(f);
}

// This functions allows to print the Log File's content
void printLog()
{

	FILE * f;
    	char c;
    	f = fopen("Log_File.log", "r");

    	if (f == NULL)
    	{	
        	printf("ERROR: Can't open file\n");
        	exit(0);
    	}

    	c = fgetc(f);
    
	while (c != EOF)
    	{
		printf ("%c", c);
        	c = fgetc(f);
    	}

    	fclose(f);

}
