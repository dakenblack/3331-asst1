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
    time_t loggedIn;
    int blocked[12];
    int blockedSize;
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
    time(&(ret.loggedIn));
    ret.blockedSize = 0;
    return ret;
}

static struct User db[24];
static int numUsers = 0;
static unsigned int blockDuration = 0;


int add_user(char* u, char* p) {
    db[numUsers] = construct_User(u,p);
    numUsers++;
}

void setBlockDuration(unsigned long d) {
    blockDuration = d;
}

unsigned long getBlockDuration() {
    return blockDuration;
}

int getNumUsers() {
    return numUsers;
}

int blockUser(int id,int toBlock) {
    int i;
    for(i=0;i<db[id].blockedSize;i++) {
        if(db[id].blocked[i] == toBlock)
            break;
    }
    if(i != db[id].blockedSize)
        return USER_ALREADY_BLACKLISTED;
    db[id].blocked[db[id].blockedSize] = toBlock;
    db[id].blockedSize++;
    return SUCCESS;
}

int unblockUser(int id, int toUnblock) {
    int i;
    for(i =0; i<db[id].blockedSize; i++) {
        if(db[id].blocked[i] == toUnblock)
            break;
    }
    if(i == db[id].blockedSize)
        return USER_NOT_BLACKLISTED;
    for(int j=i+1; j<db[id].blockedSize; j++) {
        db[id].blocked[j-1] = db[id].blocked[j];
    }
    db[id].blockedSize--;
}

int isBlocked(int from, int to) {
    int i;
    for(i=0; i<db[to].blockedSize; i++) {
        if(db[to].blocked[i] == from)
            break;
    }
    return i != db[to].blockedSize; 
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
            time_t crnt;
            time(&crnt);
            if(crnt - db[i].blockedTime > blockDuration) {
                //unblock and try again
                db[i].status = OFFLINE;
                db[i].blockedTime = 0;
                //TODO maybe figure out how to send the time left
                return find_and_login(u,p,sock);
            }
            return USER_BLOCKED;
        } else {
            if(strcmp(db[i].password,p) == 0) {
                //VALID USER
                db[i].socket = sock;
                db[i].status = ONLINE;
                time(&(db[i].lastCmd));
                db[i].loginAttempts = 0;
                time(&(db[i].loggedIn));
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

void logout(int id) {
    db[id].status = OFFLINE;
    db[id].socket = -1;
    db[id].loggedIn = 0;
}

void offlineMessage(int userId, char* m) {
    for(int i=db[userId].backlogSize; i>0; i--) {
        db[userId].backlog[i] = db[userId].backlog[i-1];
    }
    db[userId].backlog[0] = construct_message(m);
    db[userId].backlogSize++;
}

time_t getLoggedInTime(int id) {
    return db[id].loggedIn;
}

int isBacklog(int userId) {
    return db[userId].backlogSize > 0;
}

void backlogPop(int userId, char* m) {
    db[userId].backlogSize--;
    strcpy(m, db[userId].backlog[db[userId].backlogSize].msg);
}

#endif
