#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>


// *** LAUNCHER ***
// we use it to initialize all processes 
// it reads data from a configuration file and creates the necessary pipes

// main
int main()
{
	// read data from a given configuration file    
	FILE *config_file;
	// open 
    	config_file = fopen("config.txt", "r");

	// variables initialization
    	char ip[20];
	char portnumber[20];
	char referencefrequency[20];
	char waitingtime[20];  

    	// Read data from configuration file and store it in variables
    	fscanf(config_file, "%s", referencefrequency);
    	fscanf(config_file, "%s", waitingtime);
    	fscanf(config_file, "%s", ip);
    	fscanf(config_file, "%s", portnumber);

	// Initialize processes
	// pid_t data type stands for process identification 
	// it is used to represent process ids. 
	// Whenever, we want to declare a variable that is going to be deal
	// with the process ids we can use pid_t data type
    	pid_t processP; // Initialize P process
    	pid_t processL; // Initialize L process
    	pid_t processS; // Initialize S process
    	pid_t processG; // Initialize G process

    	// Using these variables, the processes can return varibles for child
    	int statusChildP;
	int statusChildL;
	int statusChildS;
	int statusChildG; 

    	// pipes declaration
	// connection process S and P
	int pipe_sp[2]; 
	// connection process G and P
    	int pipe_gp[2]; 
	// connection process P and L
    	int pipe_pl[2]; 

	if (pipe(pipe_sp) < 0)  
	{
		perror("LAUNCHER: Error when creating pipe S-P.\n");
		return -1;
	}

    	if (pipe(pipe_gp) < 0)  
	{
        	perror("LAUNCHER: Error when creating pipe G-P\n");
        	return -1;
    	}

    	if (pipe(pipe_pl) < 0)  
	{
    	    perror("LAUNCHER: Error when creating pipe P-L\n");
    	    return -1;
	}

	// Arrays to store data for the exec
	char *argP[12];
	char *argG[5];
	char *argS[4];
	char *argL[4];
	
	// Buffers used to read and write
	// Pipe S-P
	char pipe_sp_r[5]; 
	char pipe_sp_w[5];
	// Pipe G-P
    	char pipe_gp_r[5]; 
    	char pipe_gp_w[5]; 
	// Pipe P-L
	char pipe_pl_r[5]; 
	char pipe_pl_w[5];

	// from int to char
	sprintf(pipe_sp_r, "%d", pipe_sp[0]); 
	sprintf(pipe_sp_w, "%d", pipe_sp[1]);
	sprintf(pipe_gp_r, "%d", pipe_gp[0]);
	sprintf(pipe_gp_w, "%d", pipe_gp[1]);
	sprintf(pipe_pl_r, "%d", pipe_pl[0]);
	sprintf(pipe_pl_w, "%d", pipe_pl[1]);

	// Send data on process P
	argP[1] = referencefrequency; 
	argP[2] = waitingtime; 
	argP[3] = ip; 
	argP[4] = portnumber; 
	argP[5] = pipe_sp_r; 
	argP[6] = pipe_sp_w; 
	argP[7] = pipe_gp_r; 
	argP[8] = pipe_gp_w; 
	argP[9] = pipe_pl_r; 
	argP[10] = pipe_pl_w; 
	argP[11] = NULL;

   	
    	// Send data on process L
    	argL[1] = pipe_pl_r; 
    	argL[2] = pipe_pl_w; 
    	argL[3] = NULL;

    	// Send data on process S
    	argS[1] = pipe_sp_r; 
    	argS[2] = pipe_sp_w; 
    	argS[3] = NULL;

	// Send data on process G
	argG[1] = portnumber;
	argG[2] = pipe_gp_r; //read, pipe2
	argG[3] = pipe_gp_w; //write, pipe2
	argG[4] = NULL;

	// processes creation
    	processG = fork(); //Creates child process G

    	//Check on the fork
    	if (processG < 0)
    	{
        	perror("ERROR CHILD_G FAILED");
        	return -1;
    	}

	if (processG == 0)
    	{
        	fflush(stdout);
        	char *nameG; 
        	nameG = (char *)"./G";
        	argG[0] = nameG;
		//Start process G execution
        	execvp(nameG, argG); 
    	}
    	else
    	{
		// Fork system call is used for creating a new process, 
		// which is called child process. The child process runs 
		// concurrently with the process that makes the fork() call 
		// (parent process)
        	processL = fork();//Creates child process L

        	//Check if fork working
        	if (processL < 0)
        	{
            		perror("LAUNCHER: errore creating child of L");
            		return -1;
        	}
        	
		if (processL == 0)
		{
			fflush(stdout);
			char *nameL; 
		    	nameL = (char *)"./L";
		    	argL[0] = nameL; 
			//Start process L execution
		    	execvp(nameL, argL); 
		}
        	else
        	{
			// Create child process S
            		processS = fork(); 
			
			if (processS < 0)
            		{
		        	perror("ERROR CHILD_S FAILED");
		        	return -1;
            		}

			if (processS == 0)
			{
				fflush(stdout);
				char *nameS; 
				nameS = (char *)"./S";
				argS[0] = nameS;
				execvp(nameS, argS); //Start process S execution
			}
            		else  
            		{
                		processP = fork(); //Creates child process P


                		if (processP < 0)
				{
				 	perror("ERROR CHILD_P FAILED");
					return -1;
				}
		        if (processP == 0)
		        {
		            	fflush(stdout);
		            	char *nameP; 
		           	nameP = (char *)"./P";
		            	argP[0] = nameP;
		            	execvp(nameP, argP); //Start process P execution
		        }
            	}
        }


        // Wait for processes
	// The waitpid() function allows the calling thread to obtain 
	// status information for one of its child processes. 
	// The calling thread suspends processing until status information 
	// is available for the specified child process, if the options argument is 0
        waitpid(processP, &statusChildP, 0);
        waitpid(processL, &statusChildL, 0);
        waitpid(processS, &statusChildS, 0);
        waitpid(processG, &statusChildG, 0);


        // Check that each process has been terminated correctly
        if (statusChildP == 0) 
        {
            printf("\nLAUNCHER: child P terminated correctly\n");
        }

        if (statusChildP == 1)
        {
            printf("\nLAUNCHER: ERROR child P not terminated correctly\n");
        }

        if (statusChildL == 0) 
        {
            printf("\nLAUNCHER: child L terminated correctly\n");
        }

        if (statusChildL == 1)
        {
            printf("\nLAUNCHER: ERROR child L not terminated correctly\n");
        }

        if (statusChildS == 0) 
        {
            printf("\nLAUNCHER: child S terminated correctly\n");
        }

        if (statusChildS == 1)
        {
            printf("\nLAUNCHER: ERROR child S not terminated correctly\n");
        }

        if (statusChildG == 0) 
        {
            printf("\nLAUNCHER: child G terminated correctly\n");
        }

        if (statusChildG == 1)
        {
            printf("\nLAUNCHER: ERROR child G not terminated correctly\n");
        }

        exit(0);
    }
    return 0;
}     



