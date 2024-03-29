/**
 * Server Implementation of the 3331 assignemnt
 * author: Jabez Wilson(z5027406)
 *
 *
 */

#include <stdio.h>
#include "server_tcp.h"
#include "shared.h"
#include "server_methods.h"
#include "server_data.h"
#include <pthread.h>
#include <semaphore.h>

void init() {
    FILE* fd = fopen("credentials.txt","r");
    char user[12], pass[12];
    int isUser = 1;
    int uI=0,pI=0;
    int i;
    char c;
    if(fd) {
        c = getc(fd);
        while(c != EOF ) {
            char ch = (char)c;
            c = getc(fd);
            if(ch == ' ') {
                isUser = 0;
                user[uI] = '\0';
                continue;
            }
            if(ch == '\n') {
                isUser = 1;
                pass[pI] = '\0';
                printf("<%s> <%s>\n",user,pass);
                add_user(user,pass);
                pI = 0;
                uI = 0;
                continue;
            }
            if(isUser) {
                user[uI++] = ch;
            } else {
                pass[pI++] = ch;
            }
        }
        if(uI != 0 && pI != 0) {
            pass[pI] = '\0';
            printf("<%s> <%s>\n",user,pass);
            add_user(user,pass);
        }
        fclose(fd);
    }
}

int notLoggedIn[12];
int numInList = 0;

// mutex to make sure the notLoggedIn list is not corrupted
// only this resource is used between threads hence only one mutex is required
pthread_mutex_t not_logged_mutex;

/**
 * this thread continually listens for new connections and adds them to the notLoggedIn array
 *
 * It is the job of the main thread to listen for login requests from the ports in the notLoggedIn List
 */
void* thread_worker(void* arg) {
    while(1) {
        int newSocket = waitForConnection();
        pthread_mutex_lock(&not_logged_mutex);
        notLoggedIn[numInList++] = newSocket;
        /*printf("adding new socket to the list, current size: %d\n",numInList);*/
        pthread_mutex_unlock(&not_logged_mutex);
    }
}

/**
 * main thread
 */
int main(int argc, char* argv[]) {
    if(argc < 4) {
        printf("ERROR: Usage: ./server <server_port> <block_duration> <timeout>\n");
        exit(1);
    }
    pthread_mutex_init(&not_logged_mutex,NULL);
    pthread_t pth;
    int timeoutDuration = atoi(argv[3]);
    long blockDuration = atol(argv[2]);
    int port = atoi(argv[1]);
    if(port <= 0 || blockDuration <= 0 || timeoutDuration <= 0) {
        printf("given values must be numbers and larger than 0\n");
        exit(1);
    }

    init();
    setBlockDuration(blockDuration);
    initialize_tcp(port);
    pthread_create(&pth,NULL,thread_worker,NULL);
    
    while(1) {
        //checking the not logged in list for messages
        int toBeRemoved[12] = {-1,-1,-1,-1,  -1,-1,-1,-1,  -1,-1,-1,-1};
        int numInRm = 0;
        pthread_mutex_lock(&not_logged_mutex);

        //parsing the notLoggedInList, for connections that might have send a login request
        for(int i=0;i<numInList;i++) {
            char buffer[1024];
            int retVal;
            retVal = isSocketReady(notLoggedIn[i],500000);
            if( retVal > 0) {
                if(tryLogin(notLoggedIn[i])) {
                    //it worked
                    toBeRemoved[numInRm++] = i;
                }
            } else if (retVal < 0) {
                perror("INTERNAL_SERVER_ERROR\n");
            }
        }
        pthread_mutex_unlock(&not_logged_mutex);

        //removing the index from notLoggedIn
        if(numInRm > 0) {
            //unlocking and locking again to allow the other thread to continue
            pthread_mutex_lock(&not_logged_mutex);
            //first set value to 0;
            for(int i=0;i<numInRm;i++)
                notLoggedIn[toBeRemoved[i]] = 0;
            //then fill in the blanks
            for(int i=0;i<numInList;i++) {
                if(notLoggedIn[i] == 0) {
                    for(int j=i+1; j<numInList; j++) {
                        notLoggedIn[j-1] = notLoggedIn[j];
                    }
                }
            }

            numInList -= numInRm;

            pthread_mutex_unlock(&not_logged_mutex);
        }

        //this block will check for a request from any og the users that are online
        for(int i=0;i<getNumUsers();i++) {
            if(!isUserOnline(i))
                continue;
            int sk = getUserSocket(i);
            int retVal = isSocketReady(sk,500000);
            if( retVal > 0) {
                //TODO: there is no way to handle SIGPIPE right now
                resetLastCmd(i);
                connectionHandler(sk,i);
            } else if (retVal < 0) {
                perror("INTERNAL_SERVER_ERROR\n");
            } else {
                time_t crnt;
                time(&crnt);
                if((crnt - getLastCmd(i)) > timeoutDuration) {
                    sendMsgToUser(i,"User logged out due to lack of activity");
                    logout(i);
                    close(sk);
                }
            }
        }

        usleep(10000);
    }
    return 0;
}
