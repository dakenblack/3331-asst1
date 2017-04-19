/**
 * Server Implementation of the 3331 assignemnt
 * author: Jabez Wilson(z5027406)
 *
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include "shared.h"

void print_error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char* argv[]) {
    if(argc < 4) {
        printf("ERROR: Usage: ./server <server_port> <block_duration> <timeout>\n");
        exit(1);
    }
    int welcomeSocket, newSocket, cliLen, numBytes;
    struct sockaddr_in cliAddr, servAddr;

    welcomeSocket = socket(AF_INET, SOCK_STREAM,0);
    if(welcomeSocket < 0) {
        print_error("could not open socket ");
    }
    int server_port = atoi(argv[1]);

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(server_port);
    if(bind(welcomeSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        print_error("cannot bind port ");
    }
    listen(welcomeSocket,5);

    cliLen = sizeof(cliAddr);
    printf("cliLen: %d\n",cliLen);
    newSocket = accept(welcomeSocket, (struct sockaddr *) &cliAddr, &cliLen);
    printf("cliLen: %d\n",cliLen);

    if(newSocket < 0) {
        print_error("error on accept");
    }

    struct requestHeader reqHeader;
    numBytes = read(newSocket, (char*)&reqHeader, sizeof(reqHeader));
    printf("%d %d %d %d\n",reqHeader.secretKey, reqHeader.command, reqHeader.messageType, reqHeader.msgLength);
    

    close(newSocket);
    close(welcomeSocket);

    return 0;
}
