#ifndef SERVER_STUFF_H
#define SERVER_STUFF_H

#include "shared.h"
#include "server_data.h"
#include <errno.h>

/**
 * short hand function to create a repsonse object
 */
struct response getResponse(unsigned short err, unsigned short messageType, unsigned short what, unsigned long msgLength) {
    struct response ret;
    ret.secretKey = RESPONSE_KEY;
    ret.ERROR = err;
    ret.messageType = messageType;
    ret.msgLength = msgLength;
    ret.what = what;
    ret.duration = getBlockDuration();
    return ret;
}

/**
 * cusotom error and success message functions
 */
void sendErrorMsg(int fd, unsigned short err) {
    char buffer[512];
    struct response r = getResponse(err,NO_MSG,ERROR_MSG,0);
    char *p = serialize_response(buffer,r);
    int ret = send(fd,buffer,sizeof(r),MSG_NOSIGNAL);
    if( ret < 0) {
        printf("in sendErrorMsg %d\n",ret);
    }
}

void sendSuccessMsg(int fd) {
    char buffer[512];
    struct response r = getResponse(SUCCESS,NO_MSG,SUCCESS_MSG,0);
    char *p = serialize_response(buffer,r);
    int ret = send(fd,buffer,sizeof(r),MSG_NOSIGNAL);
    if( ret < 0) {
        printf("in sendSuccessMsg %d\n",ret);
    }
}

/**
 * custom read function that will also print out error messages
 *
 * @param fd file descriptor or socket
 * @param b buffer to read into
 * @param s maximum number of bytes to read
 * @param numBytes will add number of bytes read into the value pointed to by this pointer
 */
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
 * @param u userId to send message to
 * @param mess null terminated string that holds the message
 *
 */
void sendMsgToUser(int u,char* mess) {
    if(isUserOnline(u)) {
        int toSocket = getUserSocket(u);
        struct response r = getResponse(SUCCESS,RAW,MESSAGE,strlen(mess)+1);
        
        char buffer[1024];
        char *p = serialize_response(buffer,r);
        p = serialize_string(p,mess,strlen(mess) + 1);
        int ret = send(toSocket,buffer,sizeof(r) + strlen(mess) + 1,MSG_NOSIGNAL);
        if( ret < 0) {
            printf("in sendMsgToUser %d\n",ret);
        }

    } else {
        offlineMessage(u, mess);
    }
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
                usleep(1000);
                int id = getUserId(kv.key);
                while(isBacklog(id)) {
                    char m[64];
                    backlogPop(id,m);
                    sendMsgToUser(id,m);
                    //TODO flush should do the same thing right?
                    //OR maybe just wait for a byte of data back or something?
                    usleep(1000000);
                }

                return 1;
            }
        }
    }
    //assumed failure except for one case, look for the return 1
    return 0;
}

void returnHistory(int id,int sk,long timeSince) {
    int ids[12] = {-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1};
    int num = 0;
    for(int i=0;i<getNumUsers();i++) {
        if(timeSince >= 0) {
            time_t crnt;
            time(&crnt);
            if(crnt - getLoggedInTime(i) < timeSince && isUserOnline(i) && i != id) {
                ids[num++] = i; 
            }
        } else {
            if(isUserOnline(i) && i != id )
                ids[num++] = i; 
        }
    }
    struct response r = getResponse(SUCCESS,KEY,HISTORY,num*sizeof(struct key));
    char buffer[1024];
    int size = sizeof(r);
    char *ptr = serialize_response(buffer,r);
    for(int i =0;i<num;i++) {
        struct key k;
        strcpy(k.key,getUsername(ids[i]));
        ptr = serialize_key(ptr,k);
        size += sizeof(k);
    }
    int ret = send(sk,buffer,size,MSG_NOSIGNAL);
    if( ret < 0) {
        printf("in sendMsgToUser %d\n",ret);
    }
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
    char completeMsg[64];
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
                int id = getUserId(k.key);
                if(id == -1) {
                    sendErrorMsg(sk,NO_SUCH_USER);
                    return;
                }
                strcpy(completeMsg,getUsername(userId));
                strcat(completeMsg,": ");
                strcat(completeMsg,m);
                sendMsgToUser(id,completeMsg);
                break;
            case BROADCAST:
                strcpy(completeMsg,getUsername(userId));
                strcat(completeMsg,": ");
                strcat(completeMsg,ptr);
                for(int i=0;i<getNumUsers();i++) {
                    sendMsgToUser(i,completeMsg);
                }
                break;
            case HISTORY:
                if(h.messageType == NO_MSG) {
                    returnHistory(userId,sk,-1);
                } else if (h.messageType == KEY) {
                    deserialize_key(ptr,&k);
                    returnHistory(userId,sk,atol(k.key));
                } else {
                    sendErrorMsg(sk,INVALID_COMMAND);
                }
                break;
            case USER_LOGOUT:
                logout(userId);
                close(sk);
                break;
            default:
                sendErrorMsg(sk,INVALID_COMMAND);
        }
        if(h.command != USER_LOGOUT) {
            sendSuccessMsg(sk);
        }
    }
}

#endif
