#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

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

void* handle_client(void * arg){
    int sd = *((int *)arg);

    char buf[MAX_BUF];

    while(1){
        memset(buf, 0, MAX_BUF);
        if(recv(sd, buf, MAX_BUF-1, 0) <= 0) break;
        // del \n
        buf[strlen(buf)-1] = 0;

        char *command = strtok(buf, " ");
        for(int i = 0; i<strlen(command); i++) command[i] = tolower(command[i]); 
        if(strcmp(command, "set") == 0){
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
                        send(sd, "+OK\r\n", strlen("+OK\r\n"), 0);
                    }
                    else{
                        send(sd, "-ERR store full\r\n", strlen("-ERR store full\r\n"), 0);
                    }
                }
                // key update
                else{
                    strncpy(kv_store[idx].value, val, MAX_BUF);
                    send(sd, "+OK\r\n", strlen("+OK\r\n"), 0);
                }
            }
            else{
                send(sd, "-ERR wrong number of arguments\r\n", strlen("-ERR wrong number of arguments\r\n"), 0);
            }

        }
        else if(strcmp(command, "get") == 0){
            char* key = strtok(NULL, " ");
            if(key && (strtok(NULL, " ") == NULL)){
                int idx = find_key(key);
                if(idx != -1){
                    char res[MAX_BUF * 2];
                    sprintf(res, "$%zu\r\n%s\r\n", strlen(kv_store[idx].value), kv_store[idx].value);
                    send(sd, res, strlen(res), 0);
                }
                else{
                    send(sd, "$-1\r\n", strlen("$-1\r\n"), 0);
                }
            }
            else{
                send(sd, "-ERR wrong number of arguments\r\n", strlen("-ERR wrong number of arguments\r\n"), 0);
            }
        }
        else{
            send(sd, "-ERR unknown command\r\n", strlen("-ERR unknown command\r\n"), 0);
        }
    }
    close(sd);

}


int main(int argc, char* argv[]){
    if(argc != 2){
        usage();
        return -1;
    }
    
    int server_port = atoi(argv[1]);
    
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

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, (void*)handle_client, (void *)&newsd) != 0) {
            perror("pthread_create");
            close(newsd);
            continue;
        }
        pthread_detach(thread_id); 
    }

    close(sd);
    return 0;
    
}