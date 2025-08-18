#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/event.h>

#define PORT 9527
#define MAX_CLIENTS 20
#define BUFFER_SIZE 512

typedef struct {
    int fd;
    int id;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;

void broadcast(char* msg,int sender){

    char buffer[512];
    int id;
    for(int i=0;i<client_count;i++){
        if(sender==clients[i].fd){
            id = clients[i].id;
            break;
        }
    }
    snprintf(buffer, sizeof(buffer), "Client%d:%s", id, msg);

    for(int i=0;i<client_count;i++){
        if(clients[i].fd!=sender){
            write(clients[i].fd,buffer,strlen(buffer));
        }
    }
}

int main(){
    int sockfd,kq;
    struct sockaddr_in server_addr;

    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("socket");
    }

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));

    listen(sockfd,20);

    kq = kqueue();
    if(kq<0){
        perror("kq");
    }

    struct kevent kev, events[MAX_CLIENTS];

    EV_SET(&kev,sockfd,EVFILT_READ,EV_ADD|EV_ENABLE,0,0,NULL);
    kevent(kq,&kev,1,NULL,0,NULL);

    int next_id = 1;
    while(1){
        int nev = kevent(kq,NULL,0,events,MAX_CLIENTS,NULL);
        if(nev<0){
            perror("kevent");
        }
        for(int i=0;i<nev;i++){
            if(events[i].ident==sockfd){

                int cfd = accept(sockfd,NULL,NULL);
                client_t c;
                c.fd = cfd;
                c.id = next_id++;


                if(client_count<MAX_CLIENTS){

                    clients[client_count++] = c;
                    EV_SET(&kev,c.fd,EVFILT_READ,EV_ADD|EV_ENABLE,0,0,NULL);
                    kevent(kq,&kev,1,NULL,0,NULL);

                    printf("client%d connect\n",c.id);
                }
                else{
                    char msg[] = "Server full\n";
                    write(c.fd, msg, strlen(msg));
                    close(c.fd);
                }

            }else{
                char buffer[256];
                int n = read(events[i].ident,buffer,sizeof(buffer));
                if(n<=0){
                    for(int idx=0;idx<client_count;idx++){
                        if(clients[idx].fd==events[i].ident){
                            printf("client%d disconnect\n",clients[idx].id);
                            clients[idx] = clients[client_count-1];
                            client_count--;
                            close(events[i].ident);
                            break;
                        }
                    }

                }else{
                    buffer[n] = '\0';
                    printf("%s",buffer);
                    broadcast(buffer,events[i].ident);
                }
            }
        }
    }
    close(sockfd);
    return 1;

}

