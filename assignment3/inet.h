/*
 * Definitions for UDP client/server programs.
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 100

#define	SERV_UDP_PORT	9999
#define SERV_TCP_PORT 1998
#define	SERV_HOST_ADDR	"127.0.0.1" /* Change this to be your host addr: 129.130.10.43 for viper and 129.130.10.39 for cougar */