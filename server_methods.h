#ifndef SERVER_STUFF_H
#define SERVER_STUFF_H

#include "shared.h"
#include "server_data.h"


//DATA SECTION
static char buffer[1024];

//METHOD SECTION
struct response getResponse(unsigned short err, unsigned short messageType, unsigned long msgLength, unsigned long ACK) {
    struct response ret;
    ret.secretKey = RESPONSE_KEY;
    ret.ERROR = err;
    ret.messageType = messageType;
    ret.msgLength = msgLength;
    ret.ACK =ACK;
    ret.duration = 123;
    return ret;
}

void sendErrorMsg(int fd, unsigned short err) {
    struct response r = getResponse(err,NO_MSG,0,0);
    char *p = serialize_response(buffer,r);
    if(write(fd,buffer,sizeof(r)) < 0) {
        perror("something has gone wrong while sending error");
    }
}

void sendSuccessMsg(int fd,unsigned long ACK) {
    struct response r = getResponse(SUCCESS,NO_MSG,0,ACK);
    char *p = serialize_response(buffer,r);
    if(write(fd,buffer,sizeof(r)) < 0) {
        perror("something has gone wrong while sending success");
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


void connectionHandler(int newSocket) {
    struct requestHeader header;
    int error, retVal, numBytes=0;


    if(serverRead(newSocket,buffer,sizeof(header),&numBytes)) {
        //some error was encountered an error msg was already sent
        return;
    }
    char* p = deserialize_req_header(buffer,&header);
    
    //verify key
    if(header.secretKey != REQUEST_KEY) {
        sendErrorMsg(newSocket,DATA_CORRUPT);
    } else {
        switch(header.command) {
        case USER_LOGIN:
            if(header.messageType != KEY_VALUE && header.msgLength != sizeof(struct keyValue)) {
                sendErrorMsg(newSocket,INVALID_DATA);
            } else {
                struct keyValue kv;
                if(serverRead(newSocket,buffer,sizeof(kv),&numBytes))
                    return;
                deserialize_key_value(buffer,&kv);
                printf("user: %s, pass: %s\n",kv.key,kv.value);
                int retVal = find_and_login(kv.key,kv.value,newSocket);
                if(retVal) {
                    sendErrorMsg(newSocket,retVal);
                } else {
                    sendSuccessMsg(newSocket,numBytes);
                }
                numBytes = 0;
            }
            break;
        case USER_LOGOUT:
            break;
        case SEND_MESSAGE:
            break;
        case LIST_USERS:
            break;
        case HISTORY:
            break;
        case BLOCK:
            break;
        case UNBLOCK:
            break;
        default:
            sendErrorMsg(newSocket,INVALID_COMMAND);
        }
    }
}

#endif
