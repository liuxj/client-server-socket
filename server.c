/* server.c */
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/stropts.h>

#include "common.h"

int main(void)
{
	struct sockaddr_in servaddr, cliaddr;
	struct timespec tv; 
	struct timeval start;
	struct pollfd client[FDCOUNT];
    
	socklen_t cliaddr_len;
	int listenfd, new_fd;
	char buf[MAXLINE], buf_client[MAXLINE];
	int i, j, n, x, on,ret;
	int maxi, timeuse[FDCOUNT];
	
/* ===========socket============== */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
/* ==========允许地址立即使用======== */
	on = 1;
	setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
/* ==============bind============= */
	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
/* ============listen============= */
	Listen(listenfd, 20);

	printf("~~~listen connections:\n");
	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	maxi = 0;
	for(i=1;i<FDCOUNT;i++) {
		gettimeofday(&start, NULL); /* Remreber current time  */
		client[i].fd = -1;
		timeuse[i] = start.tv_sec; /* Initialise timeout of clients */
	}
	
	while(1) {
		tv.tv_sec = TIME_SEC;
		tv.tv_nsec = TIME_NSEC;

		cliaddr_len = sizeof(cliaddr);
		ret = poll(client, maxi+1, TIME_SEC*1000); /* blocking */
		if(ret == 0) {			/* Timeout of poll */
			for(i=1;i<FDCOUNT;i++) {
				if(client[i].fd != -1) {
					printf("***Timeout, connection canceled, id = %d***\n", client[i].fd);
					close(client[i].fd);
					client[i].fd = -1;
					timeuse[i] = 0;
				}
			}
			maxi = 0;
			continue;
		}
		/* check timeout*/
		for(i=1;i<FDCOUNT;i++) {
			if(client[i].fd != -1) {
				gettimeofday(&start, NULL);
				timeuse[i] = start.tv_sec - timeuse[i];
				if(timeuse[i]>TIME_SEC) {
					printf("*--**Client Timeout, id = %d\n", client[i].fd);
					close(client[i].fd);
					client[i].fd = -1;
					timeuse[i] = 0;
				}
			}
		}

		if(client[0].revents & POLLRDNORM){
			new_fd=Accept(listenfd,(struct sockaddr*)&cliaddr, cliaddr_len) ;
			if(new_fd>0) {
				for(i=1;i<FDCOUNT;i++) {
					if(client[i].fd != -1)
						continue;
					gettimeofday(&start, NULL); /* Record the time when connect. */
					timeuse[i] = start.tv_sec;
					client[i].fd = new_fd;
					break;
				}
				client[i].events = POLLRDNORM; /* make client pollrdnorm */
				if(maxi<FDCOUNT) {
					printf("new connection client %d\n", new_fd);
					if(i>maxi)
						maxi = i;
				}
				else {
					printf("***max connections arrive, disconnect***\n");
					close(new_fd);
					continue;
				}
			}
		}
		else {
			for(i=1;i<=maxi;i++) {
				if(client[i].fd == -1)
					continue;
				if(client[i].revents & (POLLERR | POLLIN | POLLRDNORM)) {
					memset(buf, 0, MAXLINE);
					n = read(client[i].fd, buf, MAXLINE);
					if(n<=0 || !strcmp(buf, EXIT_MSG)) {
						printf("client exit.id = %d\n", client[i].fd);
						close(client[i].fd);
						client[i].fd = -1;
						continue;
					}
					printf("\n---message from client---%d %d\n%s\n", client[i].fd, listenfd, buf);

					gettimeofday(&start, NULL); /* get active time for client */
					timeuse[i] = start.tv_sec;

					x = strlen(buf);
					memset(buf_client, 0, MAXLINE);
					for (j = 0; x>=0; x--, j++)
						buf_client[j] = buf[x];
					if((write(client[i].fd, buf_client, n)) == -1) {
						perror("write_1:");
						exit(1);
					}
					memset(buf, 0, MAXLINE);
					memset(buf_client, 0, MAXLINE);
					if(--ret<=0)
						break;
				}
			}

		}

	}
	close(listenfd);
	exit(0);
}
