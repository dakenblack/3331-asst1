/**
 * Client implementation of the 3331 assignment
 * author: Jabez Wilson(z5027406)
 *
 */

#include "shared.h"
#include "client_tcp.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t print_mutex, port_mutex;

/**
 * thread worker function
 *
 * expected to run indefinitely
 */
void* thread_worker(void* arg) {
    int port = getSocket(), retVal;
    char buf[1024];
    while(1) {
        pthread_mutex_lock(&port_mutex);
        retVal = isSocketReady(port,500000);
        if(retVal > 0) {
            retVal = read(port,buf,1024);
            if(retVal == 0) {
                printf("something has gone wrong on the server, exiting...\n");
                close(port);
                exit(1);
            }
            /*printf("receive %d bytes\n>",retVal);*/
        } else if(retVal < 0) {
            pthread_mutex_lock(&print_mutex);
            perror("ERROR: ");
            exit(1);
        } else {
            //socket is not ready
            pthread_mutex_unlock(&port_mutex);
            //give a chance for other threads to play
            usleep(100000);
            continue;
        }
        pthread_mutex_unlock(&port_mutex);

        if(retVal < 0) {
            perror("an error occured, exiting..");
            exit(1);
        } else if(retVal > sizeof(struct response)) { 
            struct response r;
            struct key k;
            char raw[45];
            char* p = deserialize_response(buf,&r);
            switch(r.messageType) {
                case RAW:
                    deserialize_string(p,raw,r.msgLength);
                    break;

                case KEY_AND_RAW:
                        p = deserialize_key(p,&k);
                        strcpy(raw,p);
                        break;
                default:
                        printf("error: %d",r.messageType);
            }

            pthread_mutex_lock(&print_mutex);
            switch(r.messageType) {
                case KEY_AND_RAW:
                    printf("%s: %s \n> ",k.key,raw);
                    break;
                case RAW:
                    printf("%s \n> ",raw);
                    break;
                default:
                    printf("error:");
            }
            pthread_mutex_unlock(&print_mutex);
        }
    }
}


/**
 * custom gets function that removes '\n' at the end and ' ' in the beginning
 *
 * @param str string to read into
 * @param size maximum number of bytes to read
 */
void myGets(char* str, int size) {
    fgets(str,size,stdin);
    if(str[0] == ' ') {
        for(int i=0;str[i]!='\0';i++)
            str[i] = str[i+1];
    }
    str[strlen(str)-1] = '\0';
}

/**
 * function that handles all commands to the client
 */
void commandHandler(char* command) {
    char arg1[16],arg2[16];
    if(strcmp(command,"message") == 0) {
        scanf("%s",arg1);
        myGets(arg2,16);
        int retVal = sendMessage(arg1,arg2);
        switch(retVal) {
            case SUCCESS:
                break;
            case NO_SUCH_USER:
                printf("> there is no user with that username... \n");
                break;
            case USER_BLACKLISTED:
                printf("> The recipienct has blocked your account \n");
                break;
            case SAME_USER:
                printf("> you can't send a message to yourself\n");
                break;
            default:
                printf("commandHandler: something has gone wrong: %d\n",retVal);
        }
    } else if (strcmp(command,"broadcast") == 0) {
        myGets(arg1,16);
        int retVal = sendBroadcast(arg1);
        switch(retVal) {
            case SUCCESS:
                break;
            default:
                printf("commandHelper: UNKNOWN ERROR: %d\n",retVal);
        }
    } else if (strcmp(command,"whoelse") == 0) {
        struct key* users; int num;
        int retVal = history(&users,&num);
        switch(retVal) {
            case SUCCESS:
                for(int i=0;i<num;i++) {
                    printf("%s\n",users[i].key);
                }
                free(users);
                break;
            default:
                printf("commandHelper: UNKNOWN ERROR: %d\n",retVal);
        }
    } else if (strcmp(command,"whoelsesince") == 0) {
        myGets(arg1,16);
        struct key* users; int num;
        int retVal = historySince(&users,&num,arg1);
        switch(retVal) {
            case SUCCESS:
                for(int i=0;i<num;i++) {
                    printf("%s\n",users[i].key);
                }
                free(users);
                break;
            default:
                printf("commandHelper: UNKNOWN ERROR: %d\n",retVal);
        }
    } else if (strcmp(command,"block") == 0) {
        myGets(arg1,16);
        int retVal = block_user(arg1);
        switch(retVal) {
            case SUCCESS:
                break;
            case USER_ALREADY_BLACKLISTED:
                printf("Given user has already been blacklisted\n>");
                break;
            default:
                printf("commandHelper: UNKNOWN ERROR: %d\n",retVal);
        }
    } else if (strcmp(command,"unblock") == 0) {
        myGets(arg1,16);
        int retVal = unblock_user(arg1);
        switch(retVal) {
            case SUCCESS:
                break;
            case USER_NOT_BLACKLISTED:
                printf("Given user was not previously blacklisted\n>");
                break;
            default:
                printf("commandHelper: UNKNOWN ERROR: %d\n",retVal);
        }
    } else if (strcmp(command,"logout") == 0) {
        logout();
        printf("successfully logged out, exiting...");
        exit(0);
    } else {
        printf("UNKNOWN COMMAND, exiting... ");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("ERROR: Usage: ./client <server_ip> <server_port>\n");
        exit(1);
    }

    pthread_mutex_init(&print_mutex,NULL);
    pthread_mutex_init(&port_mutex,NULL);

    int retVal;

    retVal = initialize_tcp(argv[1],atoi(argv[2]));
    switch(retVal) {
        case ERROR_UNKNOWN_HOST:
            printf("an error occured while parsing the provided host\n");
            exit(1);
        case ERROR_CANNOT_OPEN_SOCKET:
            printf("Socket could not be opened\n");
            exit(1);
        case ERROR_CANNOT_BIND_PORT:
            printf("Could not bind to port\n");
            exit(1);
        case ERROR_CANNOT_CONNECT_SERVER:
            printf("Cannot connect to server\n");
            exit(1);
        case SUCCESS:
            break;
        default:
            printf("Unkown error: %d, exiting \n",retVal);
            exit(1);
    }
    char user[12], pass[12];

    printf("> Username: ");
    scanf("%s",user);
    printf("> Password: ");
    scanf("%s",pass);
    int duration;
    int ret = login(user,pass,&duration);
    int exitFlag = 0;
    while(ret != SUCCESS && exitFlag == 0) {
        switch(ret) {
            case INVALID_CRED_2_TRIES:
            case INVALID_CRED_1_TRIES:
                printf("Enter your password again :\nPassword: ");
                scanf("%s",pass);
                break;
            case INVALID_CRED_0_TRIES:
                printf("You have been blocked for %d seconds\n",duration);
                exitFlag = 1;
                break;
            case USER_LOGGED_IN:
                printf("This user is already logged in!!\n");
                exitFlag = 1;
                break;
            case USER_BLOCKED:
                printf("User is still blocked for %d seconds", duration);
                exitFlag = 1;
                break;
            case NO_SUCH_USER:
                printf("There is no such username, please enter credentials again\n> Username: ");
                scanf("%s",user);
                printf("> Password: ");
                scanf("%s",pass);
            default:
                printf("Unknown Error: %d",ret);
                exitFlag = 1;
        }
        if(!exitFlag)
            ret = login(user,pass,&duration);
    }
    if(!exitFlag) 
        printf("> Successfully Logged in!!\n");
    else
        exit(1);
      

    pthread_t pth;
    pthread_create(&pth,NULL,thread_worker,NULL);

    char command[16];
    while(1) {
        printf("> ");
        scanf("%s",command);
        pthread_mutex_lock(&port_mutex);

        commandHandler(command);

        pthread_mutex_unlock(&port_mutex);
        //to give time to the other thread
        usleep(100000);
    }

    printf("You should never see this\n");
    while(1);
    return 0;
}
