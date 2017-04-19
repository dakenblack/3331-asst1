/**
 * Client implementation of the 3331 assignment
 * author: Jabez Wilson(z5027406)
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include "shared.h"

void print_error(char *msg)
{
    perror(msg);
    exit(1);
}

struct requestHeader getHeader(unsigned short command, unsigned short messageType, unsigned long msgLength) {
    struct requestHeader ret;
    ret.secretKey = REQUEST_KEY;
    ret.command = command;
    ret.messageType = messageType;
    ret.msgLength = msgLength;
    return ret;
}

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("ERROR: Usage: ./client <server_ip> <server_port>\n");
        exit(1);
    }

    int reqSocket, i, rc;
    struct sockaddr_in localAddr, servAddr;
    struct hostent *h;

    h = gethostbyname(argv[1]);
    if(h == NULL) 
        print_error("unknown host name");
    
    servAddr.sin_family = h->h_addrtype;
    memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    int server_port = atoi(argv[2]);
    servAddr.sin_port = htons(server_port);

    reqSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(reqSocket<0)
        print_error("cannot open socket ");
    
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);
    rc = bind(reqSocket, (struct sockaddr *) &localAddr, sizeof(localAddr));
    if(rc<0) {
        printf("%s: cannot bind port TCP %u\n",argv[0],server_port);
        print_error("error ");
    }
    /* connect to server */
    rc = connect(reqSocket, (struct sockaddr *) &servAddr, sizeof(servAddr));
    if(rc<0) {
        print_error("cannot connect ");
    }
    struct requestHeader msg = getHeader(USER_LOGIN,NO_MSG,0);
    rc = write(reqSocket, (char*)&msg, sizeof(msg));
    
    if(rc<0) {
        close(reqSocket);
        print_error("cannot send data ");
    
    }

    close(reqSocket);
    return 0;
}
