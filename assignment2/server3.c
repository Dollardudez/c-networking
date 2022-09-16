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

int main(int argc, char **argv)
{
    int                 sockfd, newsockfd, rand_int;
    unsigned int	clilen;
    struct sockaddr_in  cli_addr, serv_addr;
    pid_t               my_pid;
    struct tm           *timeptr;  /* pointer to time structure */
    time_t              clock;     /* clock value (in secs)     */
    char                s[MAX], command[MAX], buff[MAX];
    char                request;

    srand(time(0));
    setvbuf(stdout, NULL, _IOLBF, 0);
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
    printf("Server is running: TCP\n");
    for ( ; ; ) {

        /* Accept a new connection request. */
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("server: accept error\n");
            exit(1);
        }

        /* Read the request from the client. */
        read(newsockfd, &request, sizeof(char));

        printf("Received: %c \n", request);
        
        /* Generate an appropriate reply. */
        clock = time(0);
        timeptr = localtime(&clock);

        switch(request) {
            case '0':
                printf("Received SIGINT from client...\nGoodbye!\n");
                exit(0);
	            break;
            case '1':
                strftime(buff,MAX,"%T",timeptr);
                strcpy(command,"Current time: ");
                snprintf(s, MAX, "%s%s", command, buff);
	            break;

            case '2': 
                my_pid = getpid();
                sprintf(buff, "%d", my_pid);
                strcpy(command,"PID of server: ");
                snprintf(s, MAX, "%s%s", command, buff);
                break;

            case '3': 
                // more randomness
                rand_int = rand() % 31;
                sprintf(buff, "%d", rand_int);
                strcpy(command,"Random int (1-30): ");
                snprintf(s, MAX, "%s%s", command, buff);
                break;

            case '4': 
                printf("Server shutting down.");
                exit(0);
                break;

            default: strcpy(s,"Invalid request\n");
                break;
        }


        /* Send the reply to the client. */
        write(newsockfd, s, MAX);
	close(newsockfd);
    }
}
