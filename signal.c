//SIGNAL  >>  S process

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

//  error message
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// calling main
int main(int argc, char *argv[])
{		
	// variables
	int signalfd;
	char * signalfifo = "/tmp/signalfifo";

	// set FIFO
	if (mkfifo(signalfifo, S_IRUSR | S_IWUSR) != 0)
	{
		//perror("Cannot create Signal fifo. Already existing?");
	}

	char buffer[256];

	// open FIFO 
	signalfd = open(signalfifo, O_RDWR);  //read-write mode
	if (signalfd == 0) 
	{
		perror("FIFO can't be opened");
		unlink(signalfifo);
		exit(1);
	}

	while(1)
	{
		// first input signal from user
		printf("Please enter signal (choose betwenn start, stop ordump log) : ");
		bzero(buffer,256);
		fgets(buffer,256,stdin);
		
		// write input inside FIFO
		int n = write(signalfd,buffer,sizeof(buffer));
		if (n < 0) 
	 		error("ERROR when writing to socket");
		
		printf("Signal written to Signal Fifo: %s\n", buffer);
	}

	return 0;

}
