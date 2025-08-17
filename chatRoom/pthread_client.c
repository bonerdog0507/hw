#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define Server_Port 9527
#define Server_Addr "127.0.0.1"
#define BUFFER_SIZE 512

int sockfd;

void *read_thread(void *arg) {
    char buffer[BUFFER_SIZE];
    while(1) {
        int n = read(sockfd, buffer, sizeof(buffer)-1);
        if(n <= 0) {
            printf("Disconnected from server.\n");
            exit(0);
        }
        buffer[n] = '\0';
        printf("%s", buffer);
    }
}

void *write_thread(void *arg) {
    char buffer[BUFFER_SIZE];
    while(1) {
        if(fgets(buffer, sizeof(buffer), stdin) != NULL) {
            write(sockfd, buffer, strlen(buffer));
        }
    }
}

int main(){
    struct sockaddr_in server_addr;

    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("socket");
    }

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Server_Port); 
    server_addr.sin_addr.s_addr = inet_addr(Server_Addr);

    if(connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        perror("connect");
    }

    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, read_thread, NULL);
    pthread_create(&tid2, NULL, write_thread, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    close(sockfd);
    return 0;
    
}
