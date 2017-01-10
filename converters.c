#include <stdio.h>

unsigned short toShort(unsigned char* bytes)
{
    unsigned short out = bytes[0] << 8 | bytes[1];

//    printf("Bytes: %d %d\n", bytes[0], bytes[1]);
    return out;
}

unsigned int toInt(unsigned char* bytes)
{
    unsigned int out = bytes[0]<<24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
//    printf("Bytes: %d %d %d %d\n", bytes[0], bytes[1], bytes[2], bytes[3]);

    return out;
}