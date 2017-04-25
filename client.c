/**
 * Client implementation of the 3331 assignment
 * author: Jabez Wilson(z5027406)
 *
 */

#include "shared.h"
#include "client_tcp.h"
#include <pthread.h>

pthread_mutex_t print_mutex, port_mutex;

void* thread_worker(void* arg) {
    int port = getSocket();
    char buf[1024];
    while(1) {
        struct response r;
        pthread_mutex_lock(&port_mutex);
        int retVal = read(port,buf,sizeof(struct response));
        printf("receiver %d bytes\n>",retVal);
        if(retVal < 0) {
            perror("an error occured, exiting..");
            exit(1);
        } else if (retVal > sizeof(r)) {
            deserialize_response(buf,&r);
            if(r.msgLength > 0) {
                if(read(port,buf,r.msgLength) < 0 ) {
                    perror("an error occured, exiting.... ");
                    exit(2);
                } 
            }
        }
        pthread_mutex_unlock(&port_mutex);
        if(retVal > sizeof(r)) { 
            struct key k;
            char raw[45];
            char* p;
            switch(r.messageType) {
                case RAW:
                    strcpy(raw,buf);
                    break;

                case KEY_AND_RAW:
                        p = deserialize_key(buf,&k);
                        strcpy(raw,p);
                        break;
                default:
                        printf("error: %d",r.messageType);
            }

            pthread_mutex_lock(&print_mutex);
            switch(r.messageType) {
                case KEY_AND_RAW:
                    printf("%s: %s \n>",k.key,raw);
                    break;
                case RAW:
                    printf("%s \n>",raw);
                    break;
                default:
                    printf("error:");
            }
            pthread_mutex_unlock(&print_mutex);
        }
    }
}

void print_error(char *msg)
{
    perror(msg);
    exit(1);
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
    if(retVal) {
        printf("error %d \n",retVal);
        exit(1);
    }
    char user[12], pass[12];

    printf("> Username: ");
    scanf("%s",user);
    printf("> Password: ");
    scanf("%s",pass);
    int duration;
    int ret = login(user,pass,&duration);
    int exit = 0;
    while(ret != SUCCESS && exit == 0) {
        switch(ret) {
            case INVALID_CRED_2_TRIES:
            case INVALID_CRED_1_TRIES:
                printf("Enter your password again :\nPassword: ");
                scanf("%s",pass);
                break;
            case INVALID_CRED_0_TRIES:
                printf("You have been blocked for %d seconds\n",duration);
                exit = 1;
                break;
            case USER_LOGGED_IN:
                printf("This user is already logged in!!\n");
                exit = 1;
                break;
            case NO_SUCH_USER:
                printf("There is no such username, please enter credentials again\n> Username: ");
                scanf("%s",user);
                printf("> Password: ");
                scanf("%s",pass);
        }
        if(!exit)
            ret = login(user,pass,&duration);
    }
    if(exit) 
        printf("Something went really wrong??\n");
    else
        printf("Successfully Logged in!!\n");

    pthread_t pth;
    pthread_create(&pth,NULL,thread_worker,NULL);

    while(1);

    deinitialize_tcp();
    return 0;
}
