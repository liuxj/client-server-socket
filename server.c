/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>

#include "common.h"
int main(void)
{
	struct sockaddr_in servaddr, cliaddr;
    struct timeval tv;
    
	socklen_t cliaddr_len;
	int listenfd, new_fd;
	char buf[MAXLINE], buf_client[MAXLINE];
	int i, n, x, on,ret;
    fd_set clientfd;
    int maxsock;


    
/* ===========socket============== */
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket:");
        exit(1);
	}
/* ==========允许地址立即使用======== */
	on = 1;
	setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
/* ==============bind============= */
    if((bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1) {
        perror("binding:");
        exit(1);
	}
/* ============listen============= */
    listen(listenfd, 20);
    printf("~~~listen connections:\n");
    int conn_amount = 0;
    maxsock = listenfd;
    while(1) {
        FD_ZERO(&clientfd);
        FD_SET(listenfd, &clientfd);

        tv.tv_sec = TIME_SEC;
        tv.tv_usec = TIME_USEC;
        cliaddr_len = sizeof(cliaddr);

        ret = select(maxsock+1, &clientfd, NULL, NULL, &tv);
        if(ret<0) {
            perror("select:");
            break;
        }
        else if(ret == 0) {
            printf("***Timeout, connection canceled***\n");
            continue;
        }
        if(FD_ISSET(listenfd, &clientfd)) {
            new_fd=accept(listenfd,(struct sockaddr*)&cliaddr, &cliaddr_len);
            if(new_fd<=0) {
                perror("accept");
                continue;
            }
            if(conn_amount<FDCOUNT) {
                FD_SET(new_fd, &clientfd);
                printf("new connection client %d\n", new_fd);
                if(new_fd>maxsock)
                    maxsock = new_fd;
            }
            else {
                printf("max connections arrive, exit\n");
                send(new_fd, "bye", 4, 0);
                close(new_fd);
                break;
            }
        }
        memset(buf, 0, MAXLINE);
        n = read(new_fd, buf, MAXLINE);
        if(n==0) {
            printf("client exit.\n");
            continue;
        }
        if(!strcmp(buf,EXIT_MSG)) {
            printf("\n---GET \"EXIT_MSG\" from client, client exit!--\n\n");
            continue;
        }
        printf("\n---message from client---\n%s\n", buf);
        x = strlen(buf);
        memset(buf_client, 0, MAXLINE);
        for (i = 0; x>=0; x--, i++)
            buf_client[i] = buf[x];
        if((write(new_fd, buf_client, n)) == -1) {
            perror("write_1:");
            exit(1);
        }
        memset(buf, 0, MAXLINE);
        memset(buf_client, 0, MAXLINE);
    }
    close(listenfd);
    exit(0);
}
