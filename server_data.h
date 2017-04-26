#ifndef USER_H
#define USER_H

#include "shared.h"
#include <string.h>
#include <assert.h>
#include <time.h>

//DATA DEFINITIONS
struct message {
    char msg[64];
    time_t time;
};

struct message construct_message(char *m) {
    struct message ret;
    strcpy(ret.msg,m);
    time(&(ret.time));
    return ret;
}

#define ONLINE 100
#define OFFLINE 200
#define BLOCKED 300
struct User {
    char username[STRING_SIZE];
    char password[STRING_SIZE];
    int socket;
    int status;
    struct message backlog[12];
    int backlogSize;
    unsigned int loginAttempts;
    time_t lastCmd;
    time_t blockedTime;
};

struct User construct_User(char* u, char*p) {
    struct User ret;
    strcpy(ret.username,u);
    strcpy(ret.password,p);
    ret.socket = -1;
    ret.status = OFFLINE;
    ret.backlogSize = 0;
    ret.loginAttempts = 0;
    time(&(ret.lastCmd));
    return ret;
}

static struct User db[24];
static int numUsers = 0;
static unsigned int blockDuration = 0;

int add_user(char* u, char* p) {
    db[numUsers] = construct_User(u,p);
    numUsers++;
}

void setBlockDuration(unsigned int d) {
    blockDuration = d;
}

int getNumUsers() {
    return numUsers;
}

int getUserSocket(int i) {
    assert(i < numUsers);
    return db[i].socket;
}

char* getUsername(int id) {
    return db[id].username;
}

int getUserId(char* un) {
    for(int i=0;i<numUsers;i++) {
        if(strcmp(un,db[i].username) == 0)
            return i;
    }
    return -1;
}

int isUserOnline(int i) {
    return db[i].status == ONLINE;
}


int find_and_login(char* u, char* p,int sock) {
    int i;

    for(i=0;i<numUsers;i++) {
        if(strcmp(db[i].username,u) == 0) {
            break;
        }
    }
    if(i == numUsers) {
        return NO_SUCH_USER;
    } else {
        //a user was found
        if(db[i].status == ONLINE) {
            return USER_LOGGED_IN;
        } else if(db[i].status == BLOCKED) {
            // TODO: code to unblock user
            return USER_BLOCKED;
        } else {
            if(strcmp(db[i].password,p) == 0) {
                //VALID USER
                db[i].socket = sock;
                db[i].status = ONLINE;
                time(&(db[i].lastCmd));
                db[i].loginAttempts = 0;
                return SUCCESS;
            } else {
                db[i].loginAttempts ++;
                if(db[i].loginAttempts == 3) {
                    db[i].status = BLOCKED;
                    time(&(db[i].blockedTime));
                }
                return db[i].loginAttempts;
            }
        }
    }
    return SUCCESS;
}

void offlineMessage(int userId, char* m) {
    for(int i=db[userId].backlogSize; i>0; i--) {
        db[userId].backlog[i] = db[userId].backlog[i-1];
    }
    db[userId].backlog[0] = construct_message(m);
    db[userId].backlogSize++;
}

int isBacklog(int userId) {
    return db[userId].backlogSize > 0;
}

void backlogPop(int userId, char* m) {
    db[userId].backlogSize--;
    strcpy(m, db[userId].backlog[db[userId].backlogSize].msg);
}

#endif
