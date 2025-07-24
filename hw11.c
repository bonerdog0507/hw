#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define FILE_SIZE_MB 100
#define FILE_NAME "hw11.dat"

#define MEASURE_TIME(name, code_block) do {     \
    struct timeval __tv1, __tv2;                \
    gettimeofday(&__tv1, NULL);                 \
    code_block                                  \
    gettimeofday(&__tv2, NULL);                 \
    unsigned long __diff =                      \
        1000000 * (__tv2.tv_sec - __tv1.tv_sec) \
        + (__tv2.tv_usec - __tv1.tv_usec);      \
    printf("[%s]: %ld (us)\n", name, __diff);   \
} while (0); 

void seq_write(size_t block_size,FILE *fp){
    fseek(fp,0,SEEK_SET);

    char *buffer = (char*)malloc(block_size);
    if(buffer == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memset(buffer,'A',block_size);

    size_t total_bytes = FILE_SIZE_MB * 1024 * 1024;
    size_t written = 0 ;
    
    while(written < total_bytes){
        size_t bytes = fwrite(buffer,1,block_size,fp);
        if(bytes!=block_size){
            perror("fwrite");
            break;
        }
        if(fsync(fileno(fp))== -1){
            perror("fsync");
            break;
        }
        written+=bytes;
    }
    
    free(buffer);
}

void seq_read(size_t block_size,FILE *fp){
    fseek(fp,0,SEEK_SET);

    char *buffer = (char*)malloc(block_size);
    if(buffer == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t total_bytes_read = 0 ;
    size_t bytes_read = 0;
    while( (bytes_read = fread(buffer,1,block_size,fp))>0 ){
        total_bytes_read += bytes_read;
    }

    free(buffer);
}

void rand_read(size_t block_size,FILE *fp){
    char *buffer = (char*)malloc(block_size);
    if(buffer == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for(int i=0;i<50000;i++){
        off_t ofs = (rand()%(FILE_SIZE_MB*1024*1024/block_size))*block_size;
        if(fseek(fp,ofs,SEEK_SET)){
            perror("fseek");
            exit(EXIT_FAILURE);
        }
        size_t read_bytes = fread(buffer,1,block_size,fp);
        if(read_bytes!=block_size){
            perror("fread");
            break;
        }
    }
    free(buffer);
}

void rand_write1(size_t block_size,FILE *fp){
    char *buffer = (char*)malloc(block_size);
    if(buffer == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memset(buffer,'A',block_size);
    for(int i=0;i<50000;i++){
        off_t ofs = (rand()%(FILE_SIZE_MB*1024/4))*4096;
        if(fseek(fp,ofs,SEEK_SET)){
            perror("fseek");
            exit(EXIT_FAILURE);
        }
        size_t write_bytes = fwrite(buffer,1,block_size,fp);
        if(write_bytes!=block_size){
            perror("fwrite");
            break;
        }
    }
    free(buffer);
}

void rand_write2(size_t block_size,FILE *fp){
    char *buffer = (char*)malloc(block_size);
    if(buffer == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memset(buffer,'A',block_size);
    for(int i=0;i<50000;i++){
        off_t ofs = (rand()%(FILE_SIZE_MB*1024/4))*4096;
        if(fseek(fp,ofs,SEEK_SET)){
            perror("fseek");
            exit(EXIT_FAILURE);
        }
        size_t write_bytes = fwrite(buffer,1,block_size,fp);
        if(write_bytes!=block_size){
            perror("fwrite");
            break;
        }
        if(fsync(fileno(fp))== -1){
            perror("fsync");
            break;
        }
    }
    free(buffer);
}
int main(){

    FILE *fp = fopen(FILE_NAME,"w+b");
    srand(time(NULL));
    if(fp == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    MEASURE_TIME("seq_write",{seq_write(2048,fp);});
    MEASURE_TIME("seq_read",{seq_read(4096,fp);});
    MEASURE_TIME("rand_read",{rand_read(4096,fp);});
    MEASURE_TIME("rand_write1",{rand_write1(2048,fp);});
    MEASURE_TIME("rand_write2",{rand_write2(2048,fp);});    
    fclose(fp);
    return 0;
}