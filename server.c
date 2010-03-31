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
	struct timeval start;
    
	socklen_t cliaddr_len;
	int listenfd, new_fd;
	char buf[MAXLINE], buf_client[MAXLINE];
	int i, j, n, x, on,ret;
	fd_set clientfd;
	int maxsock, timeuse[FDCOUNT];
	
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
	int fd_a[FDCOUNT];
	maxsock = listenfd;
	for(i=0;i<FDCOUNT;i++) {
		fd_a[i] = -1;
		timeuse[i] = 0;
	}
	
	while(1) {
		FD_ZERO(&clientfd);
		FD_SET(listenfd, &clientfd);
        
		for(i=0;i<FDCOUNT;i++) {
			if(fd_a[i] != -1)
				FD_SET(fd_a[i], &clientfd);
		}
		cliaddr_len = sizeof(cliaddr);
		ret = select(maxsock+1, &clientfd, NULL, NULL, NULL);
		if(ret<0) {
			perror("select:");
			break;
		}
		else if(ret == 0) {
			printf("***Timeout, connection canceled***\n");
			for(i=0;i<FDCOUNT;i++) {
				if(fd_a[i] != -1) {
					close(fd_a[i]);
					FD_CLR(fd_a[i], &clientfd);
					fd_a[i] = -1;
				}
			}
			conn_amount = 0;
			continue;
		}
		else if(FD_ISSET(listenfd, &clientfd)){
			new_fd=accept(listenfd,(struct sockaddr*)&cliaddr, &cliaddr_len) ;
			if(new_fd>0) {
				for(i=0;i<FDCOUNT;i++) {
					if(fd_a[i] != -1)
						continue;
					gettimeofday(&start, NULL); /* Record the time when connect. */
					timeuse[i] = start.tv_sec;
					fd_a[i] = new_fd;
					break;
				}
				if(conn_amount<FDCOUNT) {
					FD_SET(new_fd, &clientfd);
					printf("new connection client %d\n", new_fd);
					if(new_fd>maxsock)
						maxsock = new_fd;
					conn_amount++;
				}
				else {
					printf("***max connections arrive, disconnect***\n");
					close(new_fd);
					continue;
				}
			}
		}
		else {
			for(i=0;i<FDCOUNT;i++) {
				if(fd_a[i] == -1)
					continue;
				if(!FD_ISSET(fd_a[i], &clientfd))
					continue;
				memset(buf, 0, MAXLINE);
				n = read(fd_a[i], buf, MAXLINE);


				if(n<=0 || !strcmp(buf, EXIT_MSG)) {
					printf("client exit.id = %d\n", fd_a[i]);
					close(fd_a[i]);
					FD_CLR(fd_a[i], &clientfd);
					fd_a[i] = -1;
					conn_amount--;
					continue;
				}
				printf("\n---message from client---%d\n%s\n", fd_a[i], buf);

				/* timeout judgement */		
				gettimeofday(&start, NULL); /* get time of the day */
				timeuse[i] = start.tv_sec - timeuse[i];
				if(timeuse[i] > TIME_SEC) {
					printf("***Client Timeout, id = %d\n", fd_a[i]);
					close(fd_a[i]);
					FD_CLR(fd_a[i], &clientfd);
					fd_a[i] = -1;
					timeuse[i] = 0;
					break;
				}
				else 
					timeuse[i] = start.tv_sec;
				/* ------------------------- */
				x = strlen(buf);
				memset(buf_client, 0, MAXLINE);
				for (j = 0; x>=0; x--, j++)
					buf_client[j] = buf[x];
				if((write(fd_a[i], buf_client, n)) == -1) {
					perror("write_1:");
					exit(1);
				}
				memset(buf, 0, MAXLINE);
				memset(buf_client, 0, MAXLINE);
			}
		}
	}
	close(listenfd);
	exit(0);
}
