#include "program.h"

#define SERVER_PORT 5672
#define STOP_RECV_TRANS 2

// Frame Types
#define METHOD_FRAME    1
#define HEADER_FRAME    2
#define BODY_FRAME      3
#define HEARTBEAT_FRAME 4

// General Frame Structure
// Section 4.2.3
// see https://www.rabbitmq.com/resources/specs/amqp0-9-1.pdf
struct General_Frame {
    char type;
    unsigned short channel;
    unsigned int size;
    void* payload;  /* The payload length should be equal to 'size' */
    char end;// = 0xCE;
};

// in the xml spec, these ids are acually labelled as "index"
#define CONNECTION_CLASS 10

#define START_METHOD 10
#define START_OK_METHOD 11

struct Method_Frame_Payload {
    short classId;
    short methodId;
    void arguments;
};
char* GeneralFrameToBuffer(struct General_Frame*);

int main(int argc, char** argv) {

    char recvBuff[1024];
    char buffer[] = {'A', 'M', 'Q', 'P', 0, 0, 9, 1};
    int sockfd = 0;
    int readLength, writeLength;

    struct General_Frame frm = { 
        METHOD_FRAME,
        0,
        5,
        null,
        0xCE
    };

    printf("Hello World\n");

    char* buf = GeneralFrameToBuffer(&frm);
    for (int i=0;i<13;i++)
    {
        printf("0x%x:%c\n", buf[i], buf[i]);
    }
    free(buf);
    return 0;

    // We will need to open a socket.
    if (initClientSocket(&sockfd, "127.0.0.1", SERVER_PORT) >= 0)
    {
        printf("Good connection!\n");
        // then probably a login
        // localhost 5671 guest/guest
/*
        buffer[0] = 'A';
        buffer[1] = 'M';
        buffer[2] = 'Q';
        buffer[3] = 'P';
        buffer[4] = 0;
        buffer[5] = 0;
        buffer[6] = 9;
        buffer[7] = 1;
*/
        // send some data
        printf("Writing: %s\n", buffer);
        if ((writeLength = send(sockfd, buffer, strlen(buffer), 0)) < 0)
        {
            // error("ERROR writing to socket");
            return -1;
        }

        printf("Sending a START\n");
        if ((writeLength = send(sockfd, "start", 5, 0)) < 0)
        {
            // error("ERROR writing to socket");
            return -1;
        }

        // listen for data
        printf("Reading\n");
        while ( (readLength = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0)) > 0)
        {
            printf("Read %d bytes\n", readLength);

            recvBuff[readLength] = 0;
            for (int i = 0;i<readLength;i++)
            {
                printf("Byte[%d]: %d\n", i, recvBuff[i]);

            }
            printf("%s\n", recvBuff);
            /*
            if(fputs(recvBuff, stdout) == EOF)
            {
                printf("\n Error : Fputs error\n");
            }
            */
        } 

        // Shutdown our connection
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

char* GeneralFrameToBuffer(struct General_Frame* frame)
{
    int payloadSize = frame->size;
    int bufSize = sizeof(frame->type) + sizeof(frame->channel) + sizeof(frame->size) + payloadSize + sizeof(frame->end);

    char* buf = (char*) malloc(bufSize);
    char *tmp = buf;

    if (buf == NULL)
    {
        printf("Unable to allocate buffer space for General Frame\n");
    } 
    else 
    {
        *tmp++ = frame->type;

        *tmp = frame->channel;
        tmp += sizeof(frame->channel);

        *tmp = frame->size;
        tmp += sizeof(frame->size);

        memcpy(tmp, frame->payload, frame->size);
        tmp += frame->size;

        *tmp = frame->end;
        tmp += sizeof(frame->end);
    }


    // TODO payload

    return buf;
}