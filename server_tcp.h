/**
 * header file for all things tcp for the server
 * provides and abstraction for the application
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "shared.h"

static int welcomeSocket, newSocket, cliLen, numBytes;
static struct sockaddr_in cliAddr, servAddr;

void print_error(char *msg) {
    perror(msg);
    exit(1);
}

/**
 * @returns -1: error, 0: timeout, 1: fd is ready
 */
int isSocketReady(int fd,int usec) {
    struct timeval tv;
    fd_set f_set;

    FD_ZERO(&f_set);
    FD_SET(fd, &f_set);
    tv.tv_sec = 0;
    tv.tv_usec = usec;
    return select(fd+1, &f_set, NULL, NULL, &tv);
}

int isPortOpen(int fd) {
    return write(fd,"a",1) < 0;
}


void initialize_tcp(int port, int block_durartion, int timeout) {
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

void deinitialize_tcp() {
    close(welcomeSocket);
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

