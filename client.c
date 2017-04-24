/**
 * Client implementation of the 3331 assignment
 * author: Jabez Wilson(z5027406)
 *
 */

#include "shared.h"
#include "client_tcp.h"

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

    int retVal;

    retVal = initialize_tcp(argv[1],atoi(argv[2]));
    if(retVal) {
        printf("error %d \n",retVal);
        exit(1);
    }

    retVal = login("something","some");
    if(retVal) {
        printf("something went wrong: %d",retVal);
        deinitialize_tcp();
        exit(1);
    }

    deinitialize_tcp();
    return 0;
}
