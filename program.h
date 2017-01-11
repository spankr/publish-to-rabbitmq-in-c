#ifndef __PROGRAM_H
#define __PROGRAM_H

/*
 * Important header stuff goes here.
 * Borrowed heavily from https://github.com/alanxz/rabbitmq-c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * boolean type 0 = false, true otherwise
 *
 * \since v0.1
 */
typedef int amqp_boolean_t;

/**
 * Parameters used to connect to the RabbitMQ broker
 *
 * \since v0.2
 */
struct amqp_connection_info {
  char *user;                 /**< the username to authenticate with the broker, default on most broker is 'guest' */
  char *password;             /**< the password to authenticate with the broker, default on most brokers is 'guest' */
  char *host;                 /**< the hostname of the broker */
  char *vhost;                /**< the virtual host on the broker to connect to, a good default is "/" */
  int port;                   /**< the port that the broker is listening on, default on most brokers is 5672 */
  amqp_boolean_t ssl;
};

// Frame Types
#define METHOD_FRAME    1
#define HEADER_FRAME    2
#define BODY_FRAME      3
#define HEARTBEAT_FRAME 4
#define FRAME_TERMINATOR 0xCE

// Types
// The short-string type is an octet (unsigned char) plus the array of characters it holds.
struct ShortString {
    unsigned char length;
    char* content;
};

// The long-string type is a bigger length field plus the array of bytes it holds.
struct LongString {
    unsigned int length;
    unsigned char* content;
};

struct TableEntry {
    struct ShortString name;
    unsigned char type;
    unsigned char* value;
};

struct FieldTable {
    unsigned int length;
    struct TableEntry* entries;
};

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

/*
Function Prototypes
*/

/* Initialize TCP client socket */
int initClientSocket(int *, const char *, int);

unsigned short BytesToShort(unsigned char*);
unsigned int BytesToInt(unsigned char*);
#endif