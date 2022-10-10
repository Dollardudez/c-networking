#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

struct chatter
{
    char name[21];
    int socket;
    int first_send;
};

struct chatroom
{
    char name[21];
    int socket;
    int active;
};

extern struct chatter (*chatters)[];

#define MAX_CHATROOMS 3
#define MAX_CHATTERS 3
#define MAX_ROOMS 3
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)