#include <stdio.h>
#include <string.h>
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
        perror("socket creation fail");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton fail");
        close(sd);
        return -1;
    }

    if(connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("connect fail");
        close(sd);
        return -1;
    }

    char buf[MAX_BUF];
    while(1){
        if(fgets(buf, MAX_BUF, stdin) == NULL) break;
        if(strncmp(buf, "EXIT", 4) == 0 && strlen(buf) == 4) break;

        if(send(sd, buf, strlen(buf), 0) <= 0) break;
        if(recv(sd, buf, MAX_BUF -1, 0) <= 0) break;
        
        buf[MAX_BUF] = '\0';
        if(strncmp(buf, "-ERR", 4) == 0){
            fprintf(stderr, "Server returned an error, exiting...\n");
            break;
        }
    }
    close(sd);
    return 0;
}