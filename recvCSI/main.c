/*
 * =====================================================================================
 *       Filename:  main.c
 *
 *    Description:  Here is an example for receiving CSI matrix 
 *                  Basic CSi procesing fucntion is also implemented and called
 *                  Check csi_fun.c for detail of the processing function
 *        Version:  1.0
 *
 *         Author:  Yaxiong Xie
 *         Email :  <xieyaxiongfly@gmail.com>
 *   Organization:  WANDS group @ Nanyang Technological University
 *   
 *   Copyright (c)  WANDS group @ Nanyang Technological University
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "csi_fun.h"

#define BUFSIZE 4096

int quit;
unsigned char buf_addr[BUFSIZE];
unsigned char data_buf[1500];

COMPLEX csi_matrix[3][3][114];
csi_struct*   csi_status;

void sig_handler(int signo)
{
    if (signo == SIGINT)
        quit = 1;
}

int connectServer(const char *ip, int port) {
    int fd;
    struct sockaddr_in servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip, &servAddr.sin_addr) < 0) {
        perror("inet_pton");
        return -1;
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    if(connect(fd, (struct sockaddr *)&servAddr, sizeof(servAddr))<0) {
        perror("connect");
        return -1;
    }
    printf("succeed to connect %s:%d.\n", ip, port);
    return fd;
}

int main(int argc, char* argv[])
{
    FILE*       fp;
    int         fd;
    int         i;
    int         cliFd = 0;
    int         total_msg_cnt,cnt;
    int         log_flag;
    unsigned char endian_flag;
    u_int16_t   buf_len;
    
    log_flag = 1;
    csi_status = (csi_struct*)malloc(sizeof(csi_struct));
    /* check usage */
    if (1 == argc){
        /* If you want to log the CSI for off-line processing,
         * you need to specify the name of the output file
         */
        log_flag  = 0;
        printf("/***************************************************/\n");
        printf("/*   Usage: recv_csi [output_file] [servIp:port]   */\n");
        printf("/***************************************************/\n");
    }
    if (2 <= argc){
        fp = fopen(argv[1],"w");
        if (!fp){
            printf("Fail to open <output_file>, are you root?\n");
            fclose(fp);
            return 0;
        }   
    }
    if (argc >= 3) {
        char tmp[32];
        strcpy(tmp, argv[2]);
        *strchr(tmp, ':') = 0;
        cliFd = connectServer(tmp, atoi(strchr(argv[2], ':')+1));
        if(cliFd < 0) {
            exit(1);
        }
    }

    fd = open_csi_device();
    if (fd < 0){
        perror("Failed to open the CSI device...");
        return errno;
    }
    
    printf("#Receiving data! Press Ctrl+C to quit!\n");

    quit = 0;
    total_msg_cnt = 0;
    
    while(1){
        if (1 == quit){
            return 0;
            fclose(fp);
            close_csi_device(fd);
        }

        /* keep listening to the kernel and waiting for the csi report */
        cnt = read_csi_buf(buf_addr,fd,BUFSIZE);

        if (cnt){
            total_msg_cnt += 1;

            /* fill the status struct with information about the rx packet */
            record_status(buf_addr, cnt, csi_status);

            /* 
             * fill the payload buffer with the payload
             * fill the CSI matrix with the extracted CSI value
             */
            record_csi_payload(buf_addr, csi_status, data_buf, csi_matrix); 
            
            /* Till now, we store the packet status in the struct csi_status 
             * store the packet payload in the data buffer
             * store the csi matrix in the csi buffer
             * with all those data, we can build our own processing function! 
             */
            //porcess_csi(data_buf, csi_status, csi_matrix);   
            
            printf("Recv %dth msg with rate: 0x%02x | payload len: %d\n",total_msg_cnt,csi_status->rate,csi_status->payload_len);
            
            /* log the received data for off-line processing */
            if (log_flag){
                buf_len = csi_status->buf_len;
                fwrite(&buf_len,1,2,fp);
                fwrite(buf_addr,1,buf_len,fp);
            }
            if(cliFd > 0) {
                buf_len = csi_status->buf_len;
                write(cliFd, &buf_len, sizeof(buf_len));
                write(cliFd, buf_addr, buf_len);
            }
        }
    }
    fclose(fp);
    close_csi_device(fd);
    free(csi_status);
    return 0;
}
