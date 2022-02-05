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

	int sockfd, portno, n, position;
	float receivedtoken, result;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	struct Token token;
	struct Token newtoken;
	char *ret;


	char * serverfifo = "/tmp/serverfifo";
	char * signalfifo = "/tmp/signalfifo";
	char * logfifo = "/tmp/logfifo";

	// create FIFOs
	// mkfifo(<pathname>, <permission>)
	//mkfifo(myfifo, 0666);

	// Server
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
	int serverfd, signalfd, logfd;

	// Open FIFO open () read only
	serverfd = open(serverfifo, O_RDONLY | O_NONBLOCK);
	// check for error
	if (serverfd == 0)
		perror("Server fifo can't be opened");

	signalfd = open(signalfifo, O_RDONLY | O_NONBLOCK);
	if (signalfd == 0)
		perror("Signal fifo can't be opened");

	logfd = open(logfifo, O_WRONLY);
	if (logfd == 0)
		perror("Log fifo can't be opened");

	// Create and bind the socket with the Process G
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
	// socket() create an unbound socket in a communications domain
	// it returns a file descriptor that can be used in later function calls that operate on sockets.
    portno = atoi(argv[2]); // converts the string argument str to an integer
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, the host doesn't exist\n");
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

	//  Token initialize
    printf("Enter Token value: ");
    scanf("%lf",&token.data);
    ctime(&token.t);

	// write intial Token value in the Socket  
    n = write(sockfd,&token,sizeof(token));
    if (n < 0) 
         error("ERROR writing to socket"); // check if working

	int start = 0;
	int stop = 0;

    	int DT = atoi(argv[3]); // converts the string argument str to an integer
    	int RF = atoi(argv[4]);

	while(1)
	{
		fd_set rfds;
		struct timeval tv;
		int retval;

		// Select file descriptor
		FD_ZERO(&rfds); // Initializes the file descriptor set fdset to have zero bits for all file descriptors.
		FD_SET(serverfd, &rfds); // FD_SET(fd, &fdset)   Sets the bit for the file descriptor fd in the file descriptor set fdset.
		FD_SET(signalfd, &rfds);
		/* Wait up to 2 seconds. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		// select() controls if there are data in the Signal Fifo or in the Server Fifo		
		retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
		if (retval == -1)
		       perror("select()");

		char pipebuffer[256];
		char logbuffer[256];
		bzero(logbuffer,256);

		char currenttime;

		time_t t; 


		// priority given to the signal FIFO
		// FD_ISSET Returns a non-zero value if the bit for the file descriptor fd is set in the file descriptor set pointed to by fdset, and 0 otherwise.
		if (FD_ISSET(signalfd, &rfds) == 1)
		{
			// Reading the data from the Signal Fifo
			n = read(signalfd,pipebuffer,256);
			if (n < 0) 
				error("ERROR: can't read data from signal pipe");

			printf("The received signal is: %s\n", pipebuffer);

			// If the received signal is 'start', >> Start flag to 1
			if(strcmp(pipebuffer, "start\n")==0)
			{
				start = 1; // flag
			}

			// If the received signal is 'stop', >> Start flag to 0
			else if(strcmp(pipebuffer, "stop\n")==0)
			{
				start = 0; // flag
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
				printf("File can't be openedì\n");
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
				perror("Writing ERROR\n");

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
			printf("The received token from server is: %s\n", pipebuffer);

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

			printf("The received token is: %f\n", token.data);

			// Compute the new Token
			result = token.data+(DT/1000000)* sin(2*3.14*RF*token.data) ;
			
			printf("The new computed toked is: %f\n", result);

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
		else if (FD_ISSET(serverfd, &rfds) == 0 && FD_ISSET(signalfd, &rfds) == 0)
		{
		// If no data is present inside the Pipes >> wait

			printf("There is no data in any of the pipes \n");
		}

	}
    return 0;
}

