/*-------------------------------------------------------------*/
/* server.c - sample time/date server.                         */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "inet.h"


void config_connection();

int                 sockfd, newsockfd;
unsigned int	clilen;
struct sockaddr_in  cli_addr, serv_addr;

int main(int argc, char **argv)
{
    int                 childpid, rand_int;
    pid_t               my_pid;
    struct tm           *timeptr;  /* pointer to time structure */
    time_t              clock;     /* clock value (in secs)     */
    char                s[MAX], command[MAX], buff[MAX];
    char                request;

    config_connection();
    srand(time(0));
    for ( ; ; ) {
        
        /* Read the request from the client. */
	    clilen = sizeof(cli_addr);
        recvfrom(sockfd, (char *) &request, sizeof(request), 0,
                  (struct sockaddr *)&cli_addr, &clilen);
        printf("Received: %c \n", request);

        /* Generate an appropriate reply. */
        clock = time(0);
        timeptr = localtime(&clock);

        switch(request) {
            case '0':
                printf("Received SIGINT from client...\nGoodbye!");
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
        sendto(sockfd, s, strlen(s)+1, 0,
                (struct sockaddr *) &cli_addr, clilen);
    }
}


void config_connection(){
    /* Create communication endpoint */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server: can't open datagram socket");
        exit(1);
    }

    /* Bind socket to local address */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port        = htons(SERV_UDP_PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("server: can't bind local address");
        exit(1);
    }
    printf("Server is running: UDP\n");
}
