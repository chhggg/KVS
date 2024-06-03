#include <stdio.h>

void usage(void){
    printf("USAGE : ./server <port>\n");
}

int main(int argc, char* argv[]){
    if(argc != 2){
        usage();
        return -1;
    }

    
}