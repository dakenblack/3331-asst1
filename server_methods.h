#ifndef SERVER_STUFF_H
#define SERVER_STUFF_H

#include "shared.h"
#include "server_data.h"
#include <errno.h>

//METHOD SECTION
struct response getResponse(unsigned short err, unsigned short messageType, unsigned short what, unsigned long msgLength) {
    struct response ret;
    ret.secretKey = RESPONSE_KEY;
    ret.ERROR = err;
    ret.messageType = messageType;
    ret.msgLength = msgLength;
    ret.what = what;
    ret.duration = 123;
    return ret;
}

void sendErrorMsg(int fd, unsigned short err) {
    char buffer[512];
    struct response r = getResponse(err,NO_MSG,ERROR_MSG,0);
    char *p = serialize_response(buffer,r);
    int ret = send(fd,buffer,sizeof(r),MSG_NOSIGNAL);
    if( ret < 0) {
        printf("%d\n",ret);
    }
}

void sendSuccessMsg(int fd) {
    char buffer[512];
    struct response r = getResponse(SUCCESS,NO_MSG,SUCCESS_MSG,0);
    char *p = serialize_response(buffer,r);
    int ret = send(fd,buffer,sizeof(r),MSG_NOSIGNAL);
    if( ret < 0) {
        printf("%d\n",ret);
    }
}


int serverRead(int fd,char* b, int s,int* numBytes) {
    int retVal = read(fd,b,s);
    if(retVal < 0 ) {
        perror("something went wrong while reading\n");
        sendErrorMsg(fd,INTERNAL_SERVER_ERROR);
        return 1;
    }
    *numBytes = *numBytes + retVal;
    return 0;
}

/**
 * @returns 1: it succeeded, 0: it failed
 */
int tryLogin(int sk) {
    struct requestHeader header;
    int error, retVal, numBytes=0;
    char buffer[1024];
    if(serverRead(sk,buffer,1024,&numBytes))
        return 0;

    //if a a port is closed unexpectedly select will (isPortOpen) will return true
    if(numBytes == 0)
        return 0;

    char* p = deserialize_req_header(buffer,&header);
    
    //verify key
    if(header.secretKey != REQUEST_KEY) {
        sendErrorMsg(sk,DATA_CORRUPT);
    } else {
        if(header.messageType != KEY_VALUE && header.msgLength != sizeof(struct keyValue)) {
            sendErrorMsg(sk,INVALID_DATA);
        } else {
            struct keyValue kv;
            deserialize_key_value(p,&kv);
            int retVal = find_and_login(kv.key,kv.value,sk);
            if(retVal) {
                sendErrorMsg(sk,retVal);
            } else {
                sendSuccessMsg(sk);
                return 1;
            }
        }
    }
    //assumed failure except for one case, look for the return 1
    return 0;
}

/**
 * Main Connection handler
 * handles all commadns except login
 *
 */
void connectionHandler(int sk, int userId) {
    char buffer[1024];
    struct requestHeader h;
    int retVal = read(sk,buffer,1024);
    struct key k;
    char* m;
    if(retVal < 0) {
        perror("error while reading..");
    } else if(retVal >= sizeof(h)) {
        char* ptr = deserialize_req_header(buffer,&h);
        if(h.secretKey != REQUEST_KEY) {
            sendErrorMsg(sk,DATA_CORRUPT);
            return;
        }
        switch(h.command) {
            case SEND_MESSAGE:
                m = deserialize_key(ptr,&k);
                printf("%s: %s\n",k.key,m);
                break;
            case USER_LOGOUT:
                break;
            default:
                sendErrorMsg(sk,INVALID_COMMAND);
        }
    }
    sendSuccessMsg(sk);
}

#endif
