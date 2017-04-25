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
    int ret = write(fd,buffer,sizeof(r));
    if( ret < 0) {
        perror("something has gone wrong while sending error");
    }
}

void sendSuccessMsg(int fd) {
    char buffer[512];
    struct response r = getResponse(SUCCESS,NO_MSG,SUCCESS_MSG,0);
    char *p = serialize_response(buffer,r);
    int ret = write(fd,buffer,sizeof(r));
    if( ret < 0) {
        perror("something has gone wrong while sending error");
    }
}


int serverRead(int fd,char* b, int s,int* numBytes) {
    int retVal = read(fd,b,s);
    if(retVal < 0 ) {
        perror("something went wrong while reading\n");
        sendErrorMsg(newSocket,INTERNAL_SERVER_ERROR);
        return 1;
    }
    *numBytes = *numBytes + retVal;
    return 0;
}


int tryLogin(char* buffer,int numBytes,int sk) {
    struct requestHeader header;
    int error, retVal;

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
            int retVal = find_and_login(kv.key,kv.value,newSocket);
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

#endif
