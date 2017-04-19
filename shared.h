/**
 * This file is shared between the client and the server
 * it holds the necassary information to facilitate communication between the two
 *
 * Author: Jabez Wilson (z5027406)
 *
 */

#ifndef SHARED_H
#define SHARED_H

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

//Error values
//when invalid credentials are supplied
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
#define INVALID_CLOSING 8

//messageTypes
#define NO_MSG 1
#define KEY_VALUE 2
//just assume it is a string
#define RAW 3
//the first sizeof(struct key) bytes is of the key type and the rest is raw
//used to tranmit messages to a particular user
#define KEY_AND_RAW

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
    unsigned long msgLength;
};

struct keyValue {
    char key[STRING_SIZE];
    char value[STRING_SIZE];
};

struct key {
    char key[STRING_SIZE];
};

#endif
