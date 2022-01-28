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
	int signalfd;
	char * signalfifo = "/tmp/signalfifo";


	// setting the FIFO
	if (mkfifo(signalfifo, S_IRUSR | S_IWUSR) != 0)
	{
		//perror("Cannot create Signal fifo. Already existing?");
	}

	char buffer[256];

	// opening FIFO 
	signalfd = open(signalfifo, O_RDWR);  //read-write mode
	if (signalfd == 0) {
		perror("Cannot open fifo");
		unlink(signalfifo);
		exit(1);
	}

	while(1)
	{
		// first input signal from user
		printf("Please enter the Signal start / stop / dump log : ");
		bzero(buffer,256);
		fgets(buffer,256,stdin);
		
		// writing the input inside FIFO
		int n = write(signalfd,buffer,sizeof(buffer));
		if (n < 0) 
	 		error("ERROR writing to socket");
		
		printf("Signal written to Signal Fifo: %s\n", buffer);
	}

	return 0;

}
