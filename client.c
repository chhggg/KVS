#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUF 1024

void usage(void){
    printf("USAGE : ./client <server_ip> <port_num>\n");
}

int main(int argc, char* argv[]){
    if(argc != 3){
        usage();
        return -1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1){
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sd);
        return -1;
    }

    if(connect(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        perror("connect");
        close(sd);
        return -1;
    }
    
    while(1){
        char buf[MAX_BUF];

        if(fgets(buf, MAX_BUF, stdin) == NULL) break;
        if(strcmp(buf, "EXIT\n") == 0) break;

        send(sd, buf, strlen(buf), 0);
        memset(buf, 0, MAX_BUF);

        if(recv(sd, buf, MAX_BUF, 0) <= 0) break;
        buf[MAX_BUF] = '\0';
        printf("%s", buf);

        if(strncmp(buf, "-ERR", 4) == 0){
            fprintf(stderr, "Server returned an error, exiting...\n");
            break;
        }
    }

    close(sd);
    return 0;
}