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

#define ERROR_WRITE 5
#define ERROR_NO_ACK 6

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

int deinitialize_tcp() {
    close(clientSocket);
}

int getSocket() {
    return clientSocket;
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

struct requestHeader getHeader(unsigned short command, unsigned short messageType, unsigned long msgLength) {
    struct requestHeader ret;
    ret.secretKey = REQUEST_KEY;
    ret.command = command;
    ret.messageType = messageType;
    ret.msgLength = msgLength;
    return ret;
}

int login(char user[STRING_SIZE], char pass[STRING_SIZE],int *d) {
    struct requestHeader h = getHeader(USER_LOGIN,KEY_VALUE,sizeof(struct keyValue));
    int retVal;
    struct keyValue kv;

    customStrcpy(kv.key,user,STRING_SIZE);
    customStrcpy(kv.value,pass,STRING_SIZE);

    char buf[1024];

    char* ptr = serialize_req_header(buf,h);
    ptr = serialize_key_value(ptr,kv);
    if(write(clientSocket,buf,sizeof(h) + sizeof(kv)) < 0) {
        perror("ERROR: exiting");
        exit(1);
    }

    struct response r;
    int duration = 0;
    
    //this has to be done due to the constant sending of 'a' by the server to see if the port is alive
    retVal = read(clientSocket,buf,1024);
    while(retVal < sizeof(r)) {
        if(retVal < 0) {
            perror("UNKNOWN ERROR: exiting");
            exit(1);
        }
        retVal = read(clientSocket,buf,1024);
    }
    deserialize_response(buf,&r);
    *d = r.duration;
    return r.ERROR;
}

int logout() {
    struct requestHeader h = getHeader(USER_LOGOUT,NO_MSG,0);
    int retVal;

    char buf[1024];

    char* ptr = serialize_req_header(buf,h);
    if(write(clientSocket,buf,sizeof(h)) < 0) {
        perror("what?");
        return ERROR_WRITE;
    }

    return SUCCESS;
}

