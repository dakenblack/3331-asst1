/**
 * Server Implementation of the 3331 assignemnt
 * author: Jabez Wilson(z5027406)
 *
 *
 */

#include <stdio.h>
#include "server_tcp.h"
#include "shared.h"

int main(int argc, char* argv[]) {
    if(argc < 4) {
        printf("ERROR: Usage: ./server <server_port> <block_duration> <timeout>\n");
        exit(1);
    }
    initialize(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));
    int newSocket = waitForConnection();

    struct requestHeader reqHeader;
    int error;

    numBytes = customRead(newSocket, (char*)&reqHeader, sizeof(reqHeader),&error);
    if(numBytes == 0) {
        printf("something went really wrong 1\n");
        printf("error: %d\n",error);
        close(newSocket);
        close(welcomeSocket);
        exit(1);
    }

    printf("%d %d %d %d\n",reqHeader.secretKey, reqHeader.command, reqHeader.messageType, reqHeader.msgLength);

    close(newSocket);
    close(welcomeSocket);

    return 0;
}
