#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 9527
#define MAX_CLIENTS 20
#define BUFFER_SIZE 256

typedef struct {
    int fd;
    int id;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;


void broadcast(char* msg,int sender){

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Client%d:%s", sender, msg);

    for(int i=0;i<client_count;i++){
        if(clients[i].id!=sender){
            write(clients[i].fd,buffer,strlen(buffer));
        }
    }
}

void *client_chat(void *arg){

    client_t *c = (client_t*)arg;

    char msg[] = "Type message:\n";
    char buffer[BUFFER_SIZE];

    write(c->fd,msg,strlen(msg));
    while(1){
        int n;
        n  = read(c->fd,buffer,sizeof(buffer));
        if(n<=0){
           for(int i=0; i<client_count; i++){
                if(clients[i].id == c->id){
                    clients[i] = clients[client_count-1];
                    client_count--;
                    break;
                }
            }

            printf("client%d disconnect\n",c->id);
            close(c->fd);
            break;
        }
        buffer[n] = '\0';
        printf("%s",buffer);
        broadcast(buffer,c->id);
    }
    pthread_exit(NULL);
}



int main(){
    struct sockaddr_in server_addr;
    int sockfd;

    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("socket");
        exit(1);
    }

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        perror("bind");
        close(sockfd);
        exit(1);
    }

    if(listen(sockfd,20)<0){
        perror("listen");
        close(sockfd);
        exit(1);
    }

    pthread_t tid;
    int next_id = 1;

    while(1){

        int client_fd = accept(sockfd, NULL, NULL);
        client_t *c = malloc(sizeof(client_t));
        c->fd = client_fd;
        c->id = next_id++;

        if(client_count<MAX_CLIENTS){

            clients[client_count++] = *c;
            printf("client%d connect\n",c->id);
            pthread_create(&tid, NULL, client_chat, c);
            pthread_detach(tid);

        }else{
            char msg[] = "Server full\n";
            write(client_fd, msg, strlen(msg));
            close(client_fd);
        }
        
        
    }

    close(sockfd);
    return 0;

}
