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
                continue;
            }
            if(ch == '\n') {
                isUser = 1;
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
        fclose(fd);
    }

}

int main(int argc, char* argv[]) {
    if(argc < 4) {
        printf("ERROR: Usage: ./server <server_port> <block_duration> <timeout>\n");
        exit(1);
    }

    init();
    initialize_tcp(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));
    while(1) {
        int newSocket = waitForConnection();
        connectionHandler(newSocket);
    }
    return 0;
}
