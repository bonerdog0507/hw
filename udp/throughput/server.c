#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define Server_Port 8080
int main(){
    struct sockaddr_in server_addr,client_addr;
    int sockfd,byte_recv,byte_sent;
    socklen_t client_addr_len;
    char buffer[8192];

    sockfd = socket(PF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        perror("create socket");
    }

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Server_Port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1){
        perror("bind");
        close(sockfd);
    }
    while(1){
        client_addr_len = sizeof(client_addr);
        
        byte_recv = recvfrom(sockfd,buffer,sizeof(buffer),0,(struct sockaddr*)&client_addr,&client_addr_len);
        if(byte_recv<0){
            perror("byte_recv");
        }

    }
    
    
    close(sockfd);
    return 0;
}
