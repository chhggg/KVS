#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_KEY 10
#define MAX_BUF 1024

typedef struct {
    char key[MAX_BUF];
    char value[MAX_BUF];
} kv_pair;

kv_pair kv_store[MAX_KEY];
int kv_count = 0;

void usage(void){
    printf("USAGE : ./server <port>\n");
}

int find_key(const char *key) {
    for (int i = 0; i < kv_count; i++) {
        if (strcmp(kv_store[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

int handle_client(int sd){
    char buf[MAX_BUF];
    while(1){
        if(recv(sd, buf, MAX_BUF-1, 0) <= 0) break;
        buf[MAX_BUF] = '\0';

        char *command = strtok(buf, " ");
        if(strcmp(command, "SET") == 0){
            char* key = strtok(NULL, " ");
            char* val = strtok(NULL, " ");

            if(key && val && (strtok(NULL, " ") == NULL)){
                int idx = find_key(key);
                // new key
                if(idx == -1){
                    if(kv_count < MAX_KEY){
                        strncpy(kv_store[kv_count].key, key, MAX_BUF);
                        strncpy(kv_store[kv_count].value, val, MAX_BUF);
                        kv_count++;
                        send(sd, "+OK\r\n", 5, 0);
                    }
                    else{
                        send(sd, "-ERR store full\r\n", 17, 0);
                    }
                }
                // key update
                else{
                    strncpy(kv_store[idx].value, val, MAX_BUF);
                    send(sd, "+OK\r\n", 5, 0);
                }
            }
            else{
                send(sd, "-ERR invalid command\r\n", 22, 0);
            }

        }
        else if(strcmp(command, "GET") == 0){
            char* key = strtok(NULL, "\r\n");
            if(key){
                int idx = find_key(key);
                if(idx != -1){
                    char res[MAX_BUF];
                    snprintf(res, MAX_BUF, "$%zu\r\n%s\r\n", strlen(kv_store[idx].value), kv_store[idx].value);
                    send(sd, res, strlen(res), 0);
                }
                else{
                    send(sd, "$-1\r\n", 5, 0);
                }
            }
            else{
                send(sd, "-ERR invalid command\r\n", 22, 0);
            }
        }
        else{
            send(sd, "-ERR unknown command\r\n", 22, 0);
        }
    }
    close(sd);

}


int main(int argc, char* argv[]){
    if(argc != 2){
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
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        perror("bind");
        close(sd);
        return -1;
    }

    if(listen(sd, 5) == -1){
        perror("listen");
        close(sd);
        return -1;
    }

    while (1) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int newsd = accept(sd, (struct sockaddr *)&addr, &len);
        if (newsd == -1) {
            perror("accept");
            continue;
        }

        handle_client(newsd);
    }

    close(sd);
    return 0;
    
}