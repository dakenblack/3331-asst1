/**
 * Client libraries for tcp stuff
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

#define SUCCESS 0
#define ERROR_UNKNOWN_HOST 1
#define ERROR_CANNOT_OPEN_SOCKET 2
#define ERROR_CANNOT_BIND_PORT 3
#define ERROR_CANNOT_CONNECT_SERVER 4

#define ERROR_CANNOT_SEND_DATA 5

static int clientSocket;

int initialize_tcp(char* ip, int port) {
    int i, rc;
    struct sockaddr_in localAddr, servAddr;
    struct hostent *h;

    h = gethostbyname(ip);
    if(h == NULL) 
        return ERROR_UNKNOWN_HOST;
    
    servAddr.sin_family = h->h_addrtype;
    memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servAddr.sin_port = htons(port);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket<0)
        return ERROR_CANNOT_OPEN_SOCKET;
    
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    rc = bind(clientSocket, (struct sockaddr *) &localAddr, sizeof(localAddr));
    if(rc<0) 
        return ERROR_CANNOT_BIND_PORT;
    
    /* connect to server */
    rc = connect(clientSocket, (struct sockaddr *) &servAddr, sizeof(servAddr));
    if(rc<0)
        return ERROR_CANNOT_CONNECT_SERVER;
    return SUCCESS;
}

int getSocket() {
    return clientSocket;
}
