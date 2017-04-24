#include "shared.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>



void printBin(unsigned int a, unsigned int size) {
    char* buf = malloc(size*sizeof(char));
    for(int i=0;i<size;i++) {
        if(a%2) {
            buf[i] = '1';
        } else {
            buf[i] = '0';
        }
        a = a/2;
    }
    for(int i=size-1;i>=0;i--)
        printf("%c",buf[i]);
    printf("\n");
    free(buf);
}

int main(int argc, char** argv) {
    printBin(35,8);
    printBin(240,8);
    return 0;
}
