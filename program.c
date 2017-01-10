#include "program.h"

#define SERVER_PORT 5672
#define STOP_RECV_TRANS 2

// Frame Types
#define METHOD_FRAME    1
#define HEADER_FRAME    2
#define BODY_FRAME      3
#define HEARTBEAT_FRAME 4
#define FRAME_TERMINATOR 0xCE

// General Frame Structure
// Section 4.2.3
// see https://www.rabbitmq.com/resources/specs/amqp0-9-1.pdf
// see https://www.rabbitmq.com/resources/specs/amqp0-9-1.xml
struct General_Frame {
    unsigned char type;
    unsigned short channel;
    unsigned int size;
    void* payload;  /* The payload length should be equal to 'size' */
    unsigned char end;// = 0xCE;
};

// in the xml spec, these ids are acually labelled as "index"
#define CONNECTION_CLASS 10

#define START_METHOD 10
#define START_OK_METHOD 11

struct Method_Frame_Payload {
    unsigned short classId;
    unsigned short methodId;
    void* arguments;
};
unsigned char* GeneralFrameToBuffer(struct General_Frame*);
void GetGeneralFrameFromBuffer(struct General_Frame*, unsigned char*, int);
void GetMethodPayloadFromBuffer(unsigned char*, int);
int ExtractFieldTable(unsigned char*);

int main(int argc, char** argv) {

    unsigned char recvBuff[1024];
    unsigned char buffer[] = {'A', 'M', 'Q', 'P', 0, 0, 9, 1};
    int sockfd = 0;
    int readLength, writeLength;

    struct General_Frame frm = { 
        METHOD_FRAME,
        0,
        5,
        NULL,
        0xCE
    };

    printf("Hello World\n");
/*
    char* buf = GeneralFrameToBuffer(&frm);
    for (int i=0;i<13;i++)
    {
        printf("0x%x:%c\n", buf[i], buf[i]);
    }
    free(buf);
    return 0;
*/
    // We will need to open a socket.
    if (initClientSocket(&sockfd, "127.0.0.1", SERVER_PORT) >= 0)
    {
        printf("Good connection!\n");
        // then probably a login
        // localhost 5671 guest/guest

        // send some data
        printf("Writing: [%s](%lu)\n", buffer, sizeof(buffer));
        if ((writeLength = send(sockfd, buffer, sizeof(buffer), 0)) < 0)
        {
            // error("ERROR writing to socket");
            return -1;
        }
/*
        printf("Sending a START\n");
        if ((writeLength = send(sockfd, "start", 5, 0)) < 0)
        {
            // error("ERROR writing to socket");
            return -1;
        }
*/
        // listen for data
        printf("Reading\n");
        while ( (readLength = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0)) > 0)
        {
            printf("Read %d bytes\n", readLength);

            for (int i = 0;i<35;i++)
            {
                printf("Byte[%d]: %d\n", i, recvBuff[i]);
            }
//            printf("%s\n", recvBuff);

            GetGeneralFrameFromBuffer(0L, recvBuff, readLength);
        } 
        printf("Done reading\n");

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

unsigned char* GeneralFrameToBuffer(struct General_Frame* frame)
{
    int payloadSize = frame->size;
    int bufSize = sizeof(frame->type) + sizeof(frame->channel) + sizeof(frame->size) + payloadSize + sizeof(frame->end);

    unsigned char* buf = (unsigned char*) malloc(bufSize);
    unsigned char *tmp = buf;

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

    return buf;
}

void GetGeneralFrameFromBuffer(struct General_Frame* frame, unsigned char* buf, int length)
{
    unsigned char type;
    unsigned short channel;
    unsigned int size;
    void* payload;  /* The payload length should be equal to 'size' */
    unsigned char end;// = 0xCE;
    unsigned char* tmp = buf;

    end = buf[length-1];

    if (end != FRAME_TERMINATOR)
    {
        printf("WARNING! Frame not properly terminated!\n");
    }

    type = *tmp++;

    channel = toShort(tmp);
    tmp += 2;

    size = toInt(tmp);
    tmp += 4;

    printf("General Frame:\n");
    printf("  Type: %d\n", type);
    printf("  Channel: %d\n", channel);
    printf("  Size: %d\n", size);
    printf("  End: 0x%x\n", end);

    if (type == METHOD_FRAME) {
        GetMethodPayloadFromBuffer(tmp, size);
    }
}

void GetMethodPayloadFromBuffer(unsigned char* buf, int length)
{
    unsigned char* tmp = buf;

    unsigned short classId;
    unsigned short methodId;
/*
    for (int i = 0;i<10;i++)
    {
        printf("Byte[%d]: %d\n", i, buf[i]);
    }
*/
    classId = toShort(tmp);
    tmp += sizeof(unsigned short);

    methodId = toShort(tmp);
    tmp += sizeof(unsigned short);

    printf("Method Payload:\n");
    printf("  Class Id: %d\n", classId);
    printf("  Method Id: %d\n", methodId);

    unsigned char majorVersion = *tmp++;
    unsigned char minorVersion = *tmp++;

    printf("  Major Version: %d\n", majorVersion);
    printf("  Minor Version: %d\n", minorVersion);

    // TODO server-properties
    // https://www.rabbitmq.com/amqp-0-9-1-reference.html#domain.peer-properties

    // Field Tables are long strings that contained packed key-value pairs
    // Pair:
    //   name (short string)  short string is 'unsigned short' length + char array of data (0 -> 255 chars)
    //   value type (byte)
    //   value (?)
    tmp += ExtractFieldTable(tmp);

    // mechanisms (long string)
    // long string is a 32-bit integer length value + character array
    unsigned int mechLength = toInt(tmp);
    printf("Mechanism length %d\n", mechLength);
    tmp += 4;
    for (int i=0;i<mechLength;i++)
    {
        printf("%c", *tmp++);
    }
//    char* mechanisms = (char*) malloc(mechLength);
//    memcpy(mechanisms, tmp, mechLength);
//    tmp += mechLength;
//    printf("  Mechanisms: %s\n", mechanisms);
    printf("\n");

    // locales (long string)
    unsigned int localeLength = toInt(tmp);
    printf("Locale length %d\n", localeLength);
    tmp += 4;
    for (int i=0;i<localeLength;i++)
    {
        printf("%c", *tmp++);
    }
    printf("\n");
//    char* locales = (char*) malloc(localeLength);
//    memcpy(locales, tmp, localeLength);
//    tmp += localeLength;
//    printf("  Locales: %s\n", locales);

    //free(mechanisms);
    //free(locales);
}

int ExtractFieldTable(unsigned char* buf)
{
    unsigned char* tmp = buf;
    unsigned int tableLength = toInt(tmp);
    tmp += 4;

    printf("Field Table length: %d\n", tableLength);

    return tableLength+4;
}