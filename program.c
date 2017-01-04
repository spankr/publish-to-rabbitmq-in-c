#include "program.h"

#define SERVER_PORT 5672
#define STOP_RECV_TRANS 2

int main(int argc, char** argv){

    char recvBuff[1024];
    int sockfd = 0;

    printf("Hello World\n");

    // We will need to open a socket.
    if (initClientSocket(&sockfd, "127.0.0.1", SERVER_PORT) >= 0)
    {
        printf("Good connection!\n");
        // then probably a login
        // localhost 5671 guest/guest

        if (shutdown(sockfd, STOP_RECV_TRANS) < 0)
        {
            printf("Unable to close the socket.\n");
        }
    } 
    else
    {
        printf("Bad connection.\n");
    }

    printf("Done.\n");
    return 1;
}

/**
* Client Connection logic - courtesy of http://www.thegeekstuff.com/2011/12/c-socket-programming/
*/
int initClientSocket(int * sock, const char * server_ip, int port) 
{
	struct sockaddr_in addr;

    /* Create a socket */
    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        /* Could not create socket */
        return -1;
    }

	/* Fill values into our structure used to do make a connection */
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);         /* Set port number */

    /* Server IP Address - convert to Network Byte Order */
    if(inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0)
    {
        /* inet_pton error occured */
        return -1;
    } 

	/* Make a connection to the server */
	if (connect(*sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) 
    {
        /* Connect Failed */
	    return -1;
    }

	return 0;
}