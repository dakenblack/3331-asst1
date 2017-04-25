/**
 * This file is shared between the client and the server
 * it holds the necassary information to facilitate communication between the two
 *
 * Author: Jabez Wilson (z5027406)
 *
 */

#ifndef SHARED_H
#pragma pack(0)
#define SHARED_H

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define STRING_SIZE 24

//command values
#define USER_LOGIN 1
#define USER_LOGOUT 2
#define SEND_MESSAGE 3
#define LIST_USERS 4
#define HISTORY 5
#define BROUDCAST 6
#define BLOCK 7
#define UNBLOCK 8

//What values
#define MESSAGE 1
#define USER_BROADCAST 2
#define SERVER_BROADCAST 7
#define PRIVATE_MESSAGE 3
#define ERROR_MSG 4
#define SUCCESS_MSG 0

//Error values
//when invalid credentials are supplied
#define SUCCESS 0
#define INVALID_CRED_2_TRIES 1
#define INVALID_CRED_1_TRIES 2
#define INVALID_CRED_0_TRIES 3
//if someone tries to log in as a user that is already logged in
#define USER_LOGGED_IN 4
//if this particular user is blocked for block duration
#define USER_BLOCKED 5
//if this ip address is blocked
#define IP_BLOCKED 6
#define USER_BLACKLISTED 7
#define INVALID_DATA 8
#define CHECKSUM_FAILED 30
#define READ_FAILED 40
#define WRITE_FAILED 50
#define DATA_CORRUPT 60
#define INTERNAL_SERVER_ERROR 70
#define INVALID_COMMAND 80
#define NO_SUCH_USER 90

#define SIZE_INT sizeof(int)
#define SIZE_UINT sizeof(unsigned int)
#define SIZE_USHORT sizeof(unsigned short)
#define SIZE_ULONG sizeof(unsigned long)

//messageTypes
#define NO_MSG 1
#define KEY_VALUE 2
//just assume it is a string
#define RAW 3
//the first sizeof(struct key) bytes is of the key type and the rest is raw
//used to tranmit messages to a particular user
#define KEY_AND_RAW 4

//Keys
#define REQUEST_KEY 12345
#define RESPONSE_KEY 23456

struct requestHeader {
    //magic number to make sure that the server is getting correct info
    unsigned int secretKey;
    unsigned short command;
    unsigned short messageType;
    //msg length must be a multiple of sizeof(struct ..whatever..)
    //this allows multiple things to be transmitted
    //the only exception is raw and key_and_raw there is only one instance of this being sent
    unsigned long msgLength;
};

struct response {
    //magic number to make sure that the client is getting correct info
    unsigned int secretKey;
    unsigned short ERROR;
    unsigned short messageType;
    unsigned short what;
    unsigned long msgLength;
    unsigned long duration;
};

struct keyValue {
    char key[STRING_SIZE];
    char value[STRING_SIZE];
};

struct key {
    char key[STRING_SIZE];
};

typedef unsigned char checksum;

checksum getChecksum(char* a, unsigned int size ) {
    unsigned short sum = 0;
    for(int i=0;i<size;i++) {
        sum += (unsigned short)*a;
        a++;
        while(sum & 0x0100) {
            sum = sum & 0x00ff;
            sum += 1;
        }
    }
    checksum ret = (~sum) & 0x00ff;
    return ret;
}

void customStrcpy(char *d, char*s,int size) {
    assert(size>=0);
    int flag = 0;
    for(int i=0;i<size;i++) {
        if(*s == '\0')
            flag == 1;
        if(flag)
            *d = '\0';
        else
            *d = *s;
        s++;
        d++;
    }
}

int verifyChecksum(char* a, unsigned int size, checksum c) {
    return c == getChecksum(a,size);
}

char* serialize_int(char* buf,int a) {
    char* p = (char*)&a;
    for(int i=0;i<SIZE_INT;i++,p++,buf++)
        *buf = *p;
    return buf;
}

char* serialize_uint(char* buf,unsigned int a) {
    char* p = (char*)&a;
    for(int i=0;i<SIZE_UINT;i++,p++,buf++)
        *buf = *p;
    return buf;
}
char* serialize_ushort(char* buf,unsigned short a) {
    char* p = (char*)&a;
    for(int i=0;i<SIZE_USHORT;i++,p++,buf++)
        *buf = *p;
    return buf;
}
char* serialize_ulong(char* buf,unsigned long a) {
    char* p = (char*)&a;
    for(int i=0;i<SIZE_ULONG;i++,p++,buf++)
        *buf = *p;
    return buf;
}

char* serialize_string(char* buf, char* a, int size) {
    char* p = a;
    for(int i=0;i<size;i++,p++,buf++)
        *buf = *p;
    return buf;
}

char* deserialize_int(char* buf,int* a) {
    char* p = (char*)a;
    for(int i=0;i<SIZE_INT;i++,p++,buf++)
        *p = *buf;
    return buf;
}

char* deserialize_uint(char* buf,unsigned int* a) {
    char* p = (char*)a;
    for(int i=0;i<SIZE_UINT;i++,p++,buf++)
        *p = *buf;
    return buf;
}
char* deserialize_ushort(char* buf,unsigned short* a) {
    char* p = (char*)a;
    for(int i=0;i<SIZE_USHORT;i++,p++,buf++)
        *p = *buf;
    return buf;
}
char* deserialize_ulong(char* buf,unsigned long* a) {
    char* p = (char*)a;
    for(int i=0;i<SIZE_ULONG;i++,p++,buf++)
        *p = *buf;
    return buf;
}

char* deserialize_string(char* buf, char* a, int size) {
    char* p = a;
    for(int i=0;i<size;i++,p++,buf++)
        *p = *buf;
    return buf;
}

char* serialize_req_header(char* buf, struct requestHeader a) {
    buf = serialize_uint(buf,a.secretKey);
    buf = serialize_ushort(buf,a.command);
    buf = serialize_ushort(buf,a.messageType);
    buf = serialize_ulong(buf,a.msgLength);
    return buf;
}
char* deserialize_req_header(char* buf, struct requestHeader* a) {
    buf = deserialize_uint(buf,&(a->secretKey));
    buf = deserialize_ushort(buf,&(a->command));
    buf = deserialize_ushort(buf,&(a->messageType));
    buf = deserialize_ulong(buf,&(a->msgLength));
    return buf;
}

char* serialize_key_value(char* buf, struct keyValue a) {
    buf = serialize_string(buf,a.key,STRING_SIZE);
    buf = serialize_string(buf,a.value,STRING_SIZE);
    return buf;
}

char* deserialize_key_value(char* buf, struct keyValue* a) {
    buf = deserialize_string(buf,a->key,STRING_SIZE);
    buf = deserialize_string(buf,a->value,STRING_SIZE);
    return buf;
}

char* serialize_key(char* buf, struct key a) {
    buf = serialize_string(buf,a.key,STRING_SIZE);
    return buf;
}

char* deserialize_key(char* buf, struct key* a) {
    buf = deserialize_string(buf,a->key,STRING_SIZE);
    return buf;
}

char* serialize_response(char* buf, struct response a) {
    buf = serialize_uint(buf,a.secretKey);
    buf = serialize_ushort(buf,a.ERROR);
    buf = serialize_ushort(buf,a.messageType);
    buf = serialize_ushort(buf,a.what);
    buf = serialize_ulong(buf,a.msgLength);
    buf = serialize_ulong(buf,a.duration);
    return buf;
}

char* deserialize_response(char* buf, struct response* a) {
    buf = deserialize_uint(buf,&(a->secretKey));
    buf = deserialize_ushort(buf,&(a->ERROR));
    buf = deserialize_ushort(buf,&(a->messageType));
    buf = deserialize_ushort(buf,&(a->what));
    buf = deserialize_ulong(buf,&(a->msgLength));
    buf = deserialize_ulong(buf,&(a->duration));
    return buf;
}

#endif
