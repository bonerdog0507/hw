#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define Server_Port 8080
#define Server_Addr "127.0.0.1"
#define NUM_TRIALS 100000

int main(){
    struct timeval start,end;
    struct sockaddr_in address;
    int sockfd;
    int buffer_sizes[] = {64, 256, 1024, 4096, 8192};
    int num_sizes = sizeof(buffer_sizes)/sizeof(buffer_sizes[0]);

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if(sockfd<0){
        perror("create socket");
        exit(1);
    }

    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(Server_Port);
    address.sin_addr.s_addr = inet_addr(Server_Addr);
    int addr_len = sizeof(address);

    for(int i=0;i<num_sizes;i++){
        int size = buffer_sizes[i];
        char *send_buf = malloc(size);
        memset(send_buf,'A',size);

        gettimeofday(&start,NULL);

        for(int t=0;t<NUM_TRIALS;t++){
            if(sendto(sockfd, send_buf, size, 0, (struct sockaddr*)&address, addr_len)<0){
                perror("sendto");
            }
        }

        gettimeofday(&end,NULL);

        double elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000000.0;
        double throughput = (size * NUM_TRIALS) / elapsed_sec; 

        printf("Buffer size %d bytes: Throughput = %.2f MB/s\n", 
               size, throughput / (1024*1024));

        free(send_buf);
    }

    close(sockfd);
    return 0;
}
