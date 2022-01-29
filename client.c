//CLIENT  >>  P process
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
#include <math.h>
#include <signal.h>


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

struct Token {
	double  data;
	time_t t;
};


int main(int argc, char *argv[])
{
	// declare variables
	int sockfd, portno, n, position;
	float receivedtoken, result;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	struct Token token;
	struct Token newtoken;
	char *ret;

	// paths
	char * serverfifo = "/tmp/serverfifo";
	char * signalfifo = "/tmp/signalfifo";
	char * logfifo = "/tmp/logfifo";

	// create the FIFOs
	if (mkfifo(serverfifo, 0666) != 0)
	{
		//perror("Cannot create Server fifo. Already existing?");
	}

	if (mkfifo(signalfifo, 0666) != 0)
	{
		//perror("Cannot create Signal fifo. Already existing?");
	}

	if (mkfifo(logfifo, 0666) != 0)
	{
		//perror("Cannot create Log fifo. Already existing?");
	}

	// Opening FIFOs
	int serverfd, signalfd, logfd; // needed variables

	// read only
	serverfd = open(serverfifo, O_RDONLY | O_NONBLOCK);
	if (serverfd == 0)
		perror("Cannot open server fifo");

	signalfd = open(signalfifo, O_RDONLY | O_NONBLOCK);
	if (signalfd == 0)
		perror("Cannot open signal fifo");

	// write only
	logfd = open(logfifo, O_WRONLY);
	if (logfd == 0)
		perror("Cannot open Log fifo");

	// Create and bind the Socket with Process G
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");



	//  Token initialization
    printf("Enter Token value: ");
    scanf("%lf",&token.data);
    ctime(&token.t);

	// intial Token in the Socke
    n = write(sockfd,&token,sizeof(token));
    if (n < 0) 
         error("ERROR writing to socket");

	int start = 0;
	int stop = 0;

    	int DT = atoi(argv[3]);
    	int RF = atoi(argv[4]);

	while(1)
	{
		fd_set rfds;
		struct timeval tv;
		int retval;

		//  SELECT File Descriptors

		FD_ZERO(&rfds);
		FD_SET(serverfd, &rfds);
		FD_SET(signalfd, &rfds);
		/* Wait up to 2 seconds. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		// SELECT controls if there are data in the Signal Fifo or in the Server Fifo
			
		retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
		if (retval == -1)
		       perror("select()");

		char pipebuffer[256];
		char logbuffer[256];
		bzero(logbuffer,256);

		char currenttime;

		time_t t; 


		// priority given to the signal FIFO
		if (FD_ISSET(signalfd, &rfds) == 1)
		{
			// Reading the data from the Signal Fifo
			n = read(signalfd,pipebuffer,256);
			if (n < 0) 
				error("ERROR reading from Signal Pipe");

			printf("Signal Received: %s\n", pipebuffer);

			// If the received signal is 'Start', >> Start flag to 1

			if(strcmp(pipebuffer, "start\n")==0)
			{
				start = 1;
			}

			// If the received signal is 'Stop', >> Start flag to 0
			else if(strcmp(pipebuffer, "stop\n")==0)
			{
				start = 0;
			}

			// If the received signal is 'dump log', >> Start flag to 0
			// and display the content of the log file on the screen

			if (strcmp(pipebuffer, "dump log\n") == 0) {
			FILE *fptr;
	  
			char filename[100] = "log.log", c;
		  
		  	// Open the Log File in Read mode
			fptr = fopen(filename, "r");
			if (fptr == NULL)
			{
			printf("Cannot open file \n");
			exit(0);
			}
		  	
			// Extracting the contents of the Log file until the EOF,
			// and printing them on the screen
			c = fgetc(fptr);
			while (c != EOF)
			{
				printf ("%c", c);
				c = fgetc(fptr);
			}
			start = 0;
			}

			// Writing the received Signal in the log Fifo
			time(&t);

			strcat(logbuffer, ctime(&t));
			ret = strchr(logbuffer, '\n');
			position = (ret - logbuffer);
			logbuffer[position]='\0';
			strcat(logbuffer," From S ");
			strcat(logbuffer,pipebuffer);
			strcat(logbuffer,"\n");

			int nb = write(logfd, logbuffer, sizeof(logbuffer));
			if (nb == 0)
				perror("Write error\n");

			bzero(logbuffer,256);
			bzero(pipebuffer,256);

			
		}

		// If there are data in the server FIFO
		// and the start flag is set to 1, start reading the Tokens
		// sent by G.
		else if (FD_ISSET(serverfd, &rfds) == 1 && start == 1)
		{
			// Reading the Token from the server fifo
			n = read(serverfd,&token,256);
			if (n < 0)
				error("ERROR reading from socket");


			sprintf(pipebuffer,"%f", token.data);
			printf("Token Received From Server: %s\n", pipebuffer);

			// Writing the value of the received Token inside the log Fifo
			time(&t);
			strcat(logbuffer, ctime(&t));
			ret = strchr(logbuffer, '\n');
			position = (ret - logbuffer);
			logbuffer[position]='\0';
			strcat(logbuffer," From G ");
			strcat(logbuffer,pipebuffer);
			strcat(logbuffer,"\n");

			int nb = write(logfd, logbuffer, sizeof(logbuffer));
			if (nb == 0)
				perror("Write error\n");

			bzero(logbuffer,256);

			printf("Received Token in float: %f\n", token.data);

			// Compute the new Token

            result = token.data+(DT/1000000)* sin(2*3.14*RF*token.data) ;
			
			printf("Token after computation: %f\n", result);

			usleep(DT);

			// Adding new time stamp
			newtoken.data = result;
			ctime(&newtoken.t);

			// Writing the New Token inside the socket
		    	n = write(sockfd,&newtoken,sizeof(newtoken));
	  		if (n < 0) 
	       			error("ERROR writing to socket");

			// Writing the value of new Token inside the log Fifo 
			bzero(pipebuffer,256);
			sprintf(pipebuffer,"%f", newtoken.data);
			strcat(logbuffer, ctime(&t));
			ret = strchr(logbuffer, '\n');
			position = (ret - logbuffer);
			logbuffer[position]='\0';
			strcat(logbuffer," Sent Value ");
			strcat(logbuffer,pipebuffer);
			strcat(logbuffer,"\n");

			logfd = open(logfifo, O_WRONLY);
			if (logfd == 0)
			perror("Cannot open Log fifo");

			nb = write(logfd, logbuffer, sizeof(logbuffer));
			if (nb == 0)
				perror("Write error\n");
			bzero(logbuffer,256);
			bzero(pipebuffer,256);
		}

		// If no data is present inside the Pipes >> wait
		else if (FD_ISSET(serverfd, &rfds) == 0 && FD_ISSET(signalfd, &rfds) == 0)
		{
			printf("No data in any of the PIPES \n");
		}

	}
    return 0;
}

