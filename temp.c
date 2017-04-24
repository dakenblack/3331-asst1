#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    char a[3];
    a[0] = 1;
    a[1] = 123;
    printf("%d \n",getChecksum(a,0));
    printf("%d \n",getChecksum(a,1));
    printf("%d \n",getChecksum(a,2));
    printf("1 ");
    printBin(1,8);
    printf("123 ");
    printBin(123,8);
    printBin(getChecksum(a,0),8);
    printBin(getChecksum(a,1),8);
    printBin(getChecksum(a,2),8);

    return 0;
}
