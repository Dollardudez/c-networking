/*-------------------------------------------------------------*/
/* server.c - sample iterative time/date server.               */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include "inet.h"


void setup_server_listen();

int          sockfd, newsockfd, childpid;
unsigned int clilen;
struct       sockaddr_in  cli_addr, serv_addr;

int main(int argc, char **argv)
{
    struct tm           *timeptr;  /* pointer to time structure */
    time_t              clock;     /* clock value (in secs)     */
    char                s[MAX];
    char                request;

    setup_server_listen();
    for ( ; ; ) {

        /* Accept a new connection request. */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("server: accept error");
            exit(1);
        }

        /* Read the request from the client. */
        read(newsockfd, &request, sizeof(char));

        /* Generate an appropriate reply. */
        clock = time(0);
        timeptr = localtime(&clock);

        switch(request) {

            case '1': strftime(s,MAX,"%A, %B %d, %Y",timeptr);
                break;

            case '2': strftime(s,MAX,"%T",timeptr);
                break;

            case '3': strftime(s,MAX,"%A, %B %d, %Y - %T",timeptr);
                break;

            default: strcpy(s,"Invalid request\n");
                break;
        }

        /* Send the reply to the client. */
        write(newsockfd, s, MAX);
	close(newsockfd);
    }
}

void setup_server_listen(){
    /* Create communication endpoint */
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server: can't open stream socket");
        exit(1);
    }

    /* Bind socket to local address */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port        = htons(SERV_TCP_PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("server: can't bind local address");
        exit(1);
    }

    listen(sockfd, 5);

    printf("Server is running\n");
}
