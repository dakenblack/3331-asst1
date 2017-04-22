/**
 * header file for all things tcp for the server
 * provides and abstraction for the application
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include "shared.h"

static int welcomeSocket, newSocket, cliLen, numBytes;
static struct sockaddr_in cliAddr, servAddr;

void print_error(char *msg) {
    perror(msg);
    exit(1);
}

void initialize(int port, int block_durartion, int timeout) {
    welcomeSocket = socket(AF_INET, SOCK_STREAM,0);
    if(welcomeSocket < 0) {
        print_error("could not open socket ");
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
    if(bind(welcomeSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        print_error("cannot bind port ");
    }
    listen(welcomeSocket,5);
}
    
/**
 * @pre initialize was called
 */
int waitForConnection() {
    cliLen = sizeof(cliAddr);
    int newSocket = accept(welcomeSocket, (struct sockaddr *) &cliAddr, &cliLen);
    if(newSocket < 0) {
        print_error("error on accept");
    }
    return newSocket;
}

