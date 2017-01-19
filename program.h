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

/*
Function Prototypes
*/

/* Initialize TCP client socket */
int initClientSocket(int *, const char *, int);

unsigned short BytesToShort(unsigned char*);
unsigned int BytesToInt(unsigned char*);
#endif