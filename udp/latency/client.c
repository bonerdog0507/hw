#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#define Server_Port 8080
#define Server_Addr "127.0.0.1"
#define NUM_TRIALS 100000

int main(){
    struct timeval start,end;
    struct sockaddr_in address;
    int sockfd,byte_sent,byte_recv;
    int buffer_sizes[] = {64, 256, 1024, 4096, 8192,10000};
    int num_sizes = sizeof(buffer_sizes)/sizeof(buffer_sizes[0]);

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if(sockfd<0){
        perror("create socket");
    }
  
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(Server_Port);
    address.sin_addr.s_addr = inet_addr(Server_Addr);
    int addr_len = sizeof(address);

    for(int i=0;i<num_sizes;i++){

        int size = buffer_sizes[i];
        char *send_buf = malloc(size);
        char *recv_buf = malloc(size);
        memset(send_buf,'A',size);
        
        double total_rtt = 0;
        
        for(int t=0;t<NUM_TRIALS;t++){

            gettimeofday(&start,NULL);

            if(sendto(sockfd,send_buf,size,0,(struct sockaddr*)&address,addr_len)<0){
                perror("sendto");
            }

            if(recvfrom(sockfd,recv_buf,size,0,NULL,NULL)<0){
                perror("recvfrom");
            }
            gettimeofday(&end,NULL);
            double rtt_us = (end.tv_sec - start.tv_sec) * 1000000.0 +
                            (end.tv_usec - start.tv_usec) ;
            total_rtt += rtt_us;
        }
        printf("Buffer size %d bytes: Avg RTT = %.3f us\n", 
            size, total_rtt / NUM_TRIALS);

        free(send_buf);
        free(recv_buf);

    }
    
    close(sockfd);
    return 0;
}
