#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main(){
    struct sockaddr_in server_addr, client_addr;
    int sockfd, recvfd;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[8192];

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

        while (1) {
            ssize_t n = recv(recvfd, buffer, sizeof(buffer), 0);
            if (n < 0) {
                perror("recv");
                break;
            }
        }

    }

    close(sockfd);
    return 0;
}
