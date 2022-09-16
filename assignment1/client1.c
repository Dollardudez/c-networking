/*-------------------------------------------------------------*/
/* client.c - sample time/date client.                         */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "inet.h"

int get_response(void);

int main(int argc, char **argv)
{
    int                 sockfd;
    struct sockaddr_in  cli_addr, serv_addr;
    char                s[MAX];      /* array to hold output */
    int                 response;    /* user response        */
    int                 nread;       /* number of characters */
    unsigned int	servlen;     /* length of server addr*/
    char		request;

    /* Set up the address of the server to be contacted. */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port        = htons(SERV_UDP_PORT);

    /* Set up the address of the client. */
    memset((char *) &cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family      = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(0);
    cli_addr.sin_port        = htons(0);

    /* Create a socket (an endpoint for communication). */
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("client: can't open datagram socket");
        exit(1);
    }

    /* Bind the client's socket to the client's address */
    if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        perror("client: can't bind local address");
        exit(1);
    }

    printf("%s \n",inet_ntoa(cli_addr.sin_addr));

    /* Display the menu, read user's response, and send it to the server. */
    while((response = get_response())) {
        
        /* Send the user's response to the server. */
        servlen = sizeof(serv_addr);
        request = (char)('0' + response);
        sendto (sockfd, (char *) &request, sizeof(request), 0,
                (struct sockaddr *) &serv_addr, servlen);
        if(response == 4){
            printf("Shutting down server...\n");
            printf("Goodbye!");
            break;
        }
        /* Read the server's response. */
        nread = recvfrom(sockfd, s, MAX, 0,
                (struct sockaddr *) &serv_addr, &servlen);
        if (nread > 0) {
            printf("   %s\n", s);
        } else {
            printf("Nothing read. \n");
        }
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
    printf("                1. Current Time\n");
    printf("                2. PID of the server\n");
    printf("                3. Random number between 1 and 30, inclusive\n");
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

/*
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
			return(nread);		// error, return < 0
		else if (nread == 0)
			break;				// EOF

		nleft -= nread;
		ptr   += nread;
	}
	return(nbytes - nleft);		// return >= 0
}
*/
