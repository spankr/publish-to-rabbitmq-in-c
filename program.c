#include "program.h"

#define SERVER_PORT 5672
#define STOP_RECV_TRANS 2

unsigned char* GeneralFrameToBuffer(struct General_Frame*);
void GetGeneralFrameFromBuffer(struct General_Frame*, unsigned char*, int);
void GetMethodPayloadFromBuffer(unsigned char*, int);
int ExtractFieldTable(unsigned char*);
void ExtractShortString(unsigned char** in, struct ShortString* out);
void ExtractLongString(unsigned char** in, struct LongString* out);
void BuildStartOkPayload(unsigned char** payload, int* length);

int main(int argc, char** argv) {

    unsigned char recvBuff[1024];
    unsigned char buffer[] = {'A', 'M', 'Q', 'P', 0, 0, 9, 1};
    unsigned char* tmp; /* temporary buffer */
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
        printf("Read the Connection.Start method frame\n");
        if ( (readLength = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0)) <= 0)
        {
            // error("ERROR reading from socket");
            return -1;
        }

        GetGeneralFrameFromBuffer(0L, recvBuff, readLength);

        // Now lets send a Connection.Start-OK method frame back
        unsigned char* StartOkMethodFrame;
        unsigned char* payload;
        int payloadSize;
        BuildStartOkPayload(&payload, &payloadSize);

        printf("Full Length of start-ok payload is %d\n", payloadSize);

        StartOkMethodFrame = (unsigned char*) calloc(payloadSize+8, sizeof(unsigned char));
        if (StartOkMethodFrame==0) {
            return -1;
        }
        printf("Size of StartOkMethodFrame: %d\n", payloadSize+8);
        tmp = StartOkMethodFrame;
        // Frame Type
        *tmp++ = METHOD_FRAME; // Method Frame Type
        // Channel
        *tmp++ = 0; // Channel 0, byte 1
        *tmp++ = 0; // Channel 0, byte 2
        // Payload Size
        *tmp++ = (payloadSize >> 24) & 0xFF;
        *tmp++ = (payloadSize >> 16) & 0xFF;
        *tmp++ = (payloadSize >> 8) & 0xFF;
        *tmp++ = payloadSize & 0xFF;
        // Payload
        for (int i=0;i<payloadSize;i++){
            *tmp++ = payload[i];
        }

        // Frame Terminator
        *tmp = FRAME_TERMINATOR;

        free(payload);

        // Dump our Start-Ok method frame to make sure we did it right
        GetGeneralFrameFromBuffer(0L, StartOkMethodFrame, payloadSize+8);

        if ((writeLength = send(sockfd, StartOkMethodFrame, payloadSize+8, 0)) < 0)
        {
            // error("ERROR writing to socket");
            printf("Error writing start-ok\n");
            return -1;
        }

        // listen for data
        printf("Reading the response from start-ok method frame\n");
        if ( (readLength = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0)) <= 0)
        {
            // error("ERROR reading from socket");
            printf("Error reading start-ok %d\n", readLength);
            return -1;
        }

        GetGeneralFrameFromBuffer(0L, recvBuff, readLength);


/*
        printf("Waiting for response from start-ok\n");
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
*/
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

    channel = BytesToShort(tmp);
    tmp += 2;

    size = BytesToInt(tmp);
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
    classId = BytesToShort(tmp);
    tmp += sizeof(unsigned short);

    methodId = BytesToShort(tmp);
    tmp += sizeof(unsigned short);

    printf("Method Payload:\n");
    printf("  Class Id: %d\n", classId);
    printf("  Method Id: %d\n", methodId);

    if (classId==10 && methodId == 10)
    {
        unsigned char majorVersion = *tmp++;
        unsigned char minorVersion = *tmp++;

        printf("Start Method Detected\n");
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
        struct LongString mechanism, locale;

        ExtractLongString(&tmp, &mechanism);
        printf("Mechanism [%d, %s]\n", mechanism.length, mechanism.content);
        free(mechanism.content);

        ExtractLongString(&tmp, &locale);
        printf("Locale [%d, %s]\n", locale.length, locale.content);
        free(locale.content);
    } 
    else if (classId==10 && methodId == 11)
    {
        printf("Start-Ok Method Detected\n");
        tmp += ExtractFieldTable(tmp);

        struct ShortString mechanism, locale;
        struct LongString response;

        ExtractShortString(&tmp, &mechanism);
        printf("Mechanism [%d, %s]\n", mechanism.length, mechanism.content);
        free(mechanism.content);

        ExtractLongString(&tmp, &response);
        printf("Mechanism [%d, %s]\n", response.length, response.content);
        free(mechanism.content);

        ExtractShortString(&tmp, &locale);
        printf("Locale [%d, %s]\n", locale.length, locale.content);
        free(locale.content);
    } 
    else if (classId == 10 && methodId==30)
    {
        printf("Start-Ok Method Detected\n");

    }
}

// Field Tables are long strings that contained packed key-value pairs
// Pair:
//   name (short string)  short string is 'unsigned short' length + char array of data (0 -> 255 chars)
//   value type (byte)
//   value (?)
int ExtractFieldTable(unsigned char* buf)
{
    unsigned char* tmp = buf;

    for (int i=0;i<10;i++){
        printf("[%3d] %d\n", i,buf[i]);
    }
    struct LongString table;
    ExtractLongString(&tmp, &table);
    printf("Field Table length: %d\n", table.length);
    printf("Field Table content: ");
    for (int i=0;i<table.length;i++){
        printf("[%c]", table.content[i]);
    }
    printf("\n");
    int bytesToRead = table.length;

    tmp = table.content;
    struct ShortString key;
    while (bytesToRead>0)
    {
        ExtractShortString(&tmp, &key);
        bytesToRead -= 1;
        bytesToRead -= key.length;
        printf("Key: [%d, %s]\n", key.length, key.content);

        printf("Value Type: '%c'\n", tmp[0]);
        struct LongString tmpLongString;
        switch(tmp[0]) {
            case 'F': // field type, treat as a simple long string for now
                tmp++;
                bytesToRead -= 1;
                ExtractLongString(&tmp, &tmpLongString);
                printf("Field Table length: %d\n", tmpLongString.length);
                bytesToRead -= 4;
                bytesToRead -= tmpLongString.length;
                free(tmpLongString.content);
                break;
            case 'S': // long string
                tmp++;
                bytesToRead -= 1;
                ExtractLongString(&tmp, &tmpLongString);
                printf("%s [%s]\n", key.content, tmpLongString.content);
                bytesToRead -= 4;
                bytesToRead -= tmpLongString.length;
                free(tmpLongString.content);
                break;                
            default:
                printf("WARNING: Unhandled Field Type\n");
                break;
        }
        free(key.content);
    }

    free(table.content);

    return table.length+4;
}

/**
Extract an AMQP short-string and move the pointer to the next spot in the buffer.
*/
void ExtractShortString(unsigned char** in, struct ShortString* out)
{
    out->length = **in;
    *in += 1;

    // TODO Validate malloc
    out->content = (char*) calloc(out->length+1, sizeof(char));

    memset(out->content, 0, out->length+1);
    memcpy(out->content, *in, out->length);
    *in += out->length;
}

/**
Extract an AMQP long-string and move the pointer to the next spot in the buffer.
*/
void ExtractLongString(unsigned char** in, struct LongString* out)
{
    out->length = BytesToInt(*in);
    *in += 4;

    // TODO Validate malloc
    out->content = (unsigned char*) calloc(out->length+1, sizeof(unsigned char));

    memset(out->content, 0, out->length+1);
    memcpy(out->content, *in, out->length);
    *in += out->length;
}

/*
        unsigned char StartOkMethodFrame[] = {
            1,
            0,0,
            0,0,0,33,
            0,10,
            0,11,
            0, // client properties?
            5,'P','L','A','I','N', // "PLAIN"
            0,0,0,12, 0,'g','u','e','s','t',0,'g','u','e','s','t', // in long format: 0,'guest', 0,'guest'
            5,'e','n','_','U','S', // "en_US"
            0xCE
        };
*/
// Class ID: 10
// Method ID: 11
// Mechanism: "PLAIN"
// Response: guest/guest
// Locale: "en_US"
void BuildStartOkPayload(unsigned char** payload, int* length)
{
    // class-id
    int classID = 10;
    // method-id
    int methodID = 11;
    // client properties
    unsigned char properties[] = {
        // long string size

        // long string data that contains:
        //  short string key
        // type char
        // short string value

        // these 4 bytes store the length of the rest of the array
        0,0,0,104,
        // product 'S', Lee Library
        7,'p','r','o','d','u','c','t',
        'S',
        0,0,0,11,'L','e','e',' ','L','i','b','r','a','r','y',
        // version 'S', unknown
        7,'v','e','r','s','i','o','n',
        'S',
        0,0,0,7,'u','n','k','n','o','w','n',
        // platform 'S', unknown
        8,'p','l','a','t','f','o','r','m',
        'S',
        0,0,0,7,'u','n','k','n','o','w','n',
        // information 'S', http://www.digikey.com
        11,'i','n','f','o','r','m','a','t','i','o','n',
        'S',
        0,0,0,22,'h','t','t','p',':','/','/','w','w','w','.','d','i','g','i','k','e','y','.','c','o','m'
    };
    // mechanism (short string)
    unsigned char mechanism[] = {5,'P','L','A','I','N'};
    // response (long string)
    unsigned char response[] = {0,0,0,12, 0,'g','u','e','s','t',0,'g','u','e','s','t'};
    // locale (short string)
    unsigned char locale[] = {5, 'e','n','_','U','S'};

    int fullLength = 4 +sizeof(properties) +sizeof(mechanism) +sizeof(response) +sizeof(locale);

    //printf("Full Length of start-ok is %d\n", fullLength);

    unsigned char* tmp = *payload;
    tmp = (unsigned char*) calloc(fullLength, sizeof(unsigned char));
    *length = 0;

    if (tmp != 0)
    {
        *payload = tmp;
        // Class ID
        *tmp++ = 0;
        *tmp++ = classID;

        // Method ID
        *tmp++ = 0;
        *tmp++ = methodID;

        for(int i=0;i<sizeof(properties);i++){
            *tmp++ = properties[i];
        }

        for(int i=0;i<sizeof(mechanism);i++){
            *tmp++ = mechanism[i];
        }
        for(int i=0;i<sizeof(response);i++){
            *tmp++ = response[i];
        }
        for(int i=0;i<sizeof(locale);i++){
            *tmp++ = locale[i];
        }
/*
        for (int i=0;i<fullLength;i++){
            printf("%c|", (*payload)[i]);
        }
        printf("\n");        
*/
        *length = fullLength;
    }
}