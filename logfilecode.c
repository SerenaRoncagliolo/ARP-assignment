//LOG >> L process 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main()
{
	char buffer[256];

	FILE * fp;
	fp = fopen("log.log", "a");
	char * logFifo = "/tmp/logfifo";

	int logfd = open(logFifo, O_RDONLY);

	if (logfd == 0)
		perror("Can't open Log fifo");
	
	
	while(1) // LOOP
	{
		// Opening the log File
		fp = fopen("log.log", "a");

		// Opening the logFifo in Read Only mode
		int logfd = open(logFifo, O_RDONLY);
		if (logfd == 0)
			perror("Can't open Log fifo");	
		bzero(buffer,256);
	

		// Reading data from the Log Fifo
		int n = read(logfd,buffer,256);
		if (n < 0) 
			error("ERROR when reading from logFifo");

		// Appending the data read from the FIFO, inside the log file
		fputs(buffer,fp);
		printf("Data written as log: %s\n",buffer);

		close(logfd);
		fclose(fp);
	}
	return 0;
}

