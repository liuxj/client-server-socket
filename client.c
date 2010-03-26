/* client.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"

int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr;
    char buf[MAXLINE];
    int sockfd, n;
    
    if (argc != 2) {
        printf("usage: %s [SERVER IP]\n", argv[0]);
        exit(1);
    }
/* ============sockfd============ */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket:");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(SERV_PORT);
/* ============connect============ */
    if((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1) {
        perror("server is not open or connect:");
        close(sockfd);
        exit(1);
    }
    while(1) {
        printf("\n\n---input message-->>>:\n");
        memset(buf, 0, sizeof(buf));
        scanf("%s",buf);
        if(sizeof(buf) > MAXLINE) {
            printf("\n!!! message is too large !!!\n\n");
            close(sockfd);
            exit(1);
        }
        if((write(sockfd, buf, sizeof(buf)) ) == -1) {
            perror("write_1:");
            close(sockfd);
            exit(1);
        }
        if(!strcmp(buf,EXIT_MSG)) {
            printf("\n---GET \"EXIT_MSG\", exit !--<<<\n\n");
            close(sockfd);
            exit(1);
        }
        else {
            memset(buf, 0, MAXLINE);
            n = read(sockfd, buf, MAXLINE);
            if(n == -1) {
                perror("read:");
                close(sockfd);
                exit(1);
            }
            printf("\nResponse from server:\n");
            if((write(STDOUT_FILENO, buf, n)) == -1) {
                perror("write_2:");
                close(sockfd);
                exit(1);
            }
        }
    }
    return 0;  
}

