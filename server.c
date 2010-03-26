/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "common.h"

int main(void)
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd;
	char buf[MAXLINE], buf_client[MAXLINE];
	int i, n, x, on;
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
    while(1) {
        printf("---listen connections--->>>\n");
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        while(1) {
            memset(buf, 0, MAXLINE);
            n = read(connfd, buf, MAXLINE);

            if(!strcmp(buf,EXIT_MSG)) {
                printf("\n---GET \"EXIT_MSG\" from client, client exit!--<<<\n\n");
                close(connfd);        
                break;
            }
            printf("\n---message from client--->>>\n%s\n", buf);
            x = strlen(buf);

            memset(buf_client, 0, MAXLINE);
            for (i = 0; x>=0; x--, i++)
                buf_client[i] = buf[x];
            if((write(connfd, buf_client, n)) == -1) {
                perror("write_1:");
                close(connfd);
                exit(1);
            }
            memset(buf, 0, MAXLINE);
            memset(buf_client, 0, MAXLINE);
        }
        close(connfd);
    }
	return 0;
}
