#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8080

ssize_t recv_full(int fd, char *buf, size_t len){
    size_t total = 0;
    while(total < len){
        ssize_t n = recv(fd, buf + total, len - total, 0);
        if(n <= 0) return n;
        total += n;
    }
    return total;
}

ssize_t send_full(int fd, const char *buf, size_t len){
    size_t total = 0;
    while(total < len){
        ssize_t n = send(fd, buf + total, len - total, 0);
        if(n <= 0) return n;
        total += n;
    }
    return total;
}

int main(){
    struct sockaddr_in server_addr, client_addr;
    int sockfd, recvfd;
    socklen_t client_addr_len = sizeof(client_addr);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket");
        exit(1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        exit(1);
    }

    if(listen(sockfd, 5) < 0){
        perror("listen");
        exit(1);
    }

    while(1){
        recvfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if(recvfd < 0){
            perror("accept");
            continue;
        }

        while(1){
            uint32_t len_net;
            ssize_t n = recv_full(recvfd, (char*)&len_net, sizeof(len_net));
            if(n <= 0) break;

            uint32_t len = ntohl(len_net);
            char *buf = malloc(len);
            if(!buf) break;

            if(recv_full(recvfd, buf, len) <= 0){ 
                free(buf); 
                break; 
            }
            if(send_full(recvfd, buf, len) <= 0){ 
                free(buf); 
                break; 
            }

            free(buf);
        }
        close(recvfd);
    }

    close(sockfd);
    return 0;
}
