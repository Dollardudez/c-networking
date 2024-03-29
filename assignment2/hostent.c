/*
 * Print the "hostent" information for every host whose name is
 * specified on the command line.
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<netdb.h>		/* for struct hostent */
#include	<sys/socket.h>		/* for AF_INET */
#include	<netinet/in.h>		/* for struct in_addr */
#include	<arpa/inet.h>		/* for inet_ntoa() */


/*
 * Go through a list of Internet addresses,
 * printing each one in dotted-decimal notation.
 */

void pr_inet(listptr, length)
char	**listptr;
int	length;
{
	struct in_addr *ptr;

	while ( (ptr = (struct in_addr *) *listptr++) != NULL)
		printf("	Internet address: %s\n", inet_ntoa(*ptr));
}

int main(argc, argv)
int	argc;
char	**argv;
{
	register char		*ptr;
	register struct hostent	*hostptr;

	while (--argc > 0) {
		ptr = *++argv;
		if ( (hostptr = gethostbyname(ptr)) == NULL) {
			printf("gethostbyname error for host: %s\n", ptr);
			continue;
		}
		printf("official host name: %s\n", hostptr->h_name);

		/* go through the list of aliases */
		while ( (ptr = *(hostptr->h_aliases)) != NULL) {
			printf("	alias: %s\n", ptr);
			hostptr->h_aliases++;
		}
		printf("	addr type = %d, addr length = %d\n",
				hostptr->h_addrtype, hostptr->h_length);

		switch (hostptr->h_addrtype) {
		case AF_INET:
			pr_inet(hostptr->h_addr_list, hostptr->h_length);
			break;

		default:
			printf("unknown address type \n");
			break;
		}
	}
}
