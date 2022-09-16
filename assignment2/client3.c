/*-------------------------------------------------------------*/
/* client.c - sample time/date client.                         */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include "inet.h"

int get_response(void);
int readn(int, char *, int);
void setup_server_addr();
void connect_to_server();

int sockfd;
struct sockaddr_in  serv_addr;

int main(int argc, char **argv)
{
    char                s[MAX];      /* array to hold output */
    int                 response;    /* user response        */
    int                 nread;       /* number of characters */

    setup_server_addr();
    /* Display the menu, read user's response, and send it to the server. */
    while((response = get_response()) != 4) {

        connect_to_server();
        
        sprintf(s,"%d",response);

        /* Send the user's request to the server. */
        write (sockfd, s, sizeof(char));

        /* Read the server's reply. */
        nread = readn (sockfd, s, MAX);
        if (nread > 0) {
			printf("   %s\n", s);
	} else {
		printf("Nothing read. \n");
	}
        close(sockfd);
    }
    exit(0);  /* Exit if response is 4  */
}

/* Display menu and retrieve user's response */
int get_response()
{
    int choice;

    printf("===========================================\n");
    printf("                   Menu: \n");
    printf("-------------------------------------------\n");
    printf("                1. Date\n");
    printf("                2. Time\n");
    printf("                3. Both\n");
    printf("                4. Quit\n");
    printf("-------------------------------------------\n");
    printf("               Choice (1-4):");
    scanf("%1d",&choice);
    printf("===========================================\n");
    return(choice);
}

/*
 * Read up to "n" bytes from a descriptor.
 * Use in place of read() when fd is a stream socket.
 */

int
readn(fd, ptr, nbytes)
register int	fd;
register char	*ptr;
register int	nbytes;
{
	int	nleft, nread;

	nleft = nbytes;
	while (nleft > 0) {
		nread = read(fd, ptr, nleft);
		if (nread < 0)
			return(nread);		/* error, return < 0 */
		else if (nread == 0)
			break;			/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(nbytes - nleft);		/* return >= 0 */
}

void setup_server_addr(){
    /* Set up the address of the server to be contacted. */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port        = htons(SERV_TCP_PORT);
}

void connect_to_server(){
    /* Create a socket (an endpoint for communication). */
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client: can't open stream socket");
        exit(1);
    }

    /* Connect to the server. */
    if (connect(sockfd, (struct sockaddr *) &serv_addr,
        sizeof(serv_addr)) < 0) {
        perror("client: can't connect to server");
        exit(1);
    }
}

