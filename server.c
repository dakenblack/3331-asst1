/**
 * Server Implementation of the 3331 assignemnt
 * author: Jabez Wilson(z5027406)
 *
 *
 */

#include <stdio.h>
#include "server_tcp.h"
#include "shared.h"

struct response getResponse(unsigned short err, unsigned short messageType, unsigned long msgLength, unsigned long ACK) {
    struct response ret;
    ret.secretKey = RESPONSE_KEY;
    ret.ERROR = err;
    ret.messageType = messageType;
    ret.msgLength = msgLength;
    ret.ACK =ACK;
    return ret;
}

int main(int argc, char* argv[]) {
    if(argc < 4) {
        printf("ERROR: Usage: ./server <server_port> <block_duration> <timeout>\n");
        exit(1);
    }
    initialize(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));
    int newSocket = waitForConnection();

    struct requestHeader reqHeader;
    int error, retVal, numBytes=0;

    char buf[1024];

    retVal = read(newSocket,buf,sizeof(reqHeader));
    if(retVal < 0 ) {
        perror("something went wrong\n");
        exit(1);
    }

    char* p = deserialize_req_header(buf,&reqHeader);
    printf("%d %d %d %d\n",reqHeader.secretKey, reqHeader.command, reqHeader.messageType, reqHeader.msgLength);


    close(newSocket);
    close(welcomeSocket);

    return 0;
}
