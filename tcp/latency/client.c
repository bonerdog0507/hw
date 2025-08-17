#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdint.h>

#define Server_Port 8080
#define Server_Addr "127.0.0.1"
#define NUM_TRIALS 100000

ssize_t send_full(int fd, const char *buf, size_t len){
    size_t total = 0;
    while(total < len){
        ssize_t n = send(fd, buf + total, len - total, 0);
        if(n <= 0) return n;
        total += n;
    }
    return total;
}

ssize_t recv_full(int fd, char *buf, size_t len){
    size_t total = 0;
    while(total < len){
        ssize_t n = recv(fd, buf + total, len - total, 0);
        if(n <= 0) return n;
        total += n;
    }
    return total;
}

int main(){
    struct timeval start, end;
    struct sockaddr_in address;
    int sockfd;
    int buffer_sizes[] = {64, 256, 1024, 4096, 8192};
    int num_sizes = sizeof(buffer_sizes)/sizeof(buffer_sizes[0]);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket");
        exit(1);
    }

    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(Server_Port);
    address.sin_addr.s_addr = inet_addr(Server_Addr);

    if(connect(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("connect");
        close(sockfd);
        exit(1);
    }

    for(int i = 0; i < num_sizes; i++){
        int size = buffer_sizes[i];
        char *send_buf = malloc(size);
        char *recv_buf = malloc(size);
        if(!send_buf || !recv_buf){
            perror("malloc");
            exit(1);
        }
        memset(send_buf,'A',size);

        double total_rtt = 0;

        for(int t = 0; t < NUM_TRIALS; t++){
            uint32_t len_net = htonl(size);

            gettimeofday(&start,NULL);

            if(send_full(sockfd, (char*)&len_net, sizeof(len_net)) <= 0){
                perror("send length");
                break;
            }

            if(send_full(sockfd, send_buf, size) <= 0){
                perror("send data");
                break;
            }

            if(recv_full(sockfd, recv_buf, size) <= 0){
                perror("recv data");
                break;
            }

            gettimeofday(&end,NULL);
            double rtt_us = (end.tv_sec - start.tv_sec)*1000000.0 +
                            (end.tv_usec - start.tv_usec);
            total_rtt += rtt_us;
        }

        printf("Buffer size %d bytes: Avg RTT = %.3f us\n", size, total_rtt / NUM_TRIALS);

        free(send_buf);
        free(recv_buf);
    }

    close(sockfd);
    return 0;
}
