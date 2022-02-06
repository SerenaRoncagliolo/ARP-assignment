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
    	pid_t P_process; // Initialize P process
    	pid_t L_process; // Initialize L process
    	pid_t S_process; // Initialize S process
    	pid_t G_process; // Initialize G process

    	// Using these variables, the processes can return varibles for child
    	int status_P_child;
	int status_L_child;
	int status_S_child;
	int status_G_child; 

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
	char *argumentP[12];
	char *argumentG[5];
	char *argumentS[4];
	char *argumentL[4];
	
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
	argumentP[1] = referencefrequency; 
	argumentP[2] = waitingtime; 
	argumentP[3] = ip; 
	argumentP[4] = portnumber; 
	argumentP[5] = pipe_sp_r; 
	argumentP[6] = pipe_sp_w; 
	argumentP[7] = pipe_gp_r; 
	argumentP[8] = pipe_gp_w; 
	argumentP[9] = pipe_pl_r; 
	argumentP[10] = pipe_pl_w; 
	argumentP[11] = NULL;

   	
    	// Send data on process L
    	argumentL[1] = pipe_pl_r; 
    	argumentL[2] = pipe_pl_w; 
    	argumentL[3] = NULL;

    	// Send data on process S
    	argumentS[1] = pipe_sp_r; 
    	argumentS[2] = pipe_sp_w; 
    	argumentS[3] = NULL;

	// Send data on process G
	argumentG[1] = portnumber;
	argumentG[2] = pipe_gp_r; //read, pipe2
	argumentG[3] = pipe_gp_w; //write, pipe2
	argumentG[4] = NULL;

	// processes creation
    	G_process = fork(); //Creates child process G

    	//Check on the fork
    	if (G_process < 0)
    	{
        	perror("ERROR CHILD_G FAILED");
        	return -1;
    	}

	if (G_process == 0)
    	{
        	fflush(stdout);
        	char *nameG; 
        	nameG = (char *)"./G";
        	argumentG[0] = nameG;
		//Start process G execution
        	execvp(nameG, argumentG); 
    	}
    	else
    	{
		// Fork system call is used for creating a new process, 
		// which is called child process. The child process runs 
		// concurrently with the process that makes the fork() call 
		// (parent process)
        	L_process = fork();//Creates child process L

        	//Check if fork working
        	if (L_process < 0)
        	{
            		perror("LAUNCHER: errore creating child of L");
            		return -1;
        	}
        	
		if (L_process == 0)
		{
			fflush(stdout);
			char *nameL; 
		    	nameL = (char *)"./L";
		    	argumentL[0] = nameL; 
			//Start process L execution
		    	execvp(nameL, argumentL); 
		}
        	else
        	{
			// Create child process S
            		S_process = fork(); 
			
			if (S_process < 0)
            		{
		        	perror("ERROR CHILD_S FAILED");
		        	return -1;
            		}

			if (S_process == 0)
			{
				fflush(stdout);
				char *nameS; 
				nameS = (char *)"./S";
				argumentS[0] = nameS;
				execvp(nameS, argumentS); //Start process S execution
			}
            		else  
            		{
                		P_process = fork(); //Creates child process P


                		if (P_process < 0)
				{
				 	perror("ERROR CHILD_P FAILED");
					return -1;
				}
		        if (P_process == 0)
		        {
		            	fflush(stdout);
		            	char *nameP; 
		           	nameP = (char *)"./P";
		            	argumentP[0] = nameP;
		            	execvp(nameP, argumentP); //Start process P execution
		        }
            	}
        }


        // Wait for processes
	// The waitpid() function allows the calling thread to obtain 
	// status information for one of its child processes. 
	// The calling thread suspends processing until status information 
	// is available for the specified child process, if the options argument is 0
        waitpid(P_process, &status_P_child, 0);
        waitpid(L_process, &status_L_child, 0);
        waitpid(S_process, &status_S_child, 0);
        waitpid(G_process, &status_G_child, 0);


        // Check that each process has been terminated correctly
        if (status_P_child == 0) 
        {
            printf("\nLAUNCHER: child P terminated correctly\n");
        }

        if (status_P_child == 1)
        {
            printf("\nLAUNCHER: ERROR child P not terminated correctly\n");
        }

        if (status_L_child == 0) 
        {
            printf("\nLAUNCHER: child L terminated correctly\n");
        }

        if (status_L_child == 1)
        {
            printf("\nLAUNCHER: ERROR child L not terminated correctly\n");
        }

        if (status_S_child == 0) 
        {
            printf("\nLAUNCHER: child S terminated correctly\n");
        }

        if (status_S_child == 1)
        {
            printf("\nLAUNCHER: ERROR child S not terminated correctly\n");
        }

        if (status_G_child == 0) 
        {
            printf("\nLAUNCHER: child G terminated correctly\n");
        }

        if (status_G_child == 1)
        {
            printf("\nLAUNCHER: ERROR child G not terminated correctly\n");
        }

        exit(0);
    }
    return 0;
}     



