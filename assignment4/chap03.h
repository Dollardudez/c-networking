#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

struct chatter
{
    char name[1024];
    int socket;
    int first_send;
};

struct chatroom
{
    char name[30];
    int socket;
    int active;
};

extern struct chatter (*chatters)[];



#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)



#include <stdio.h>
#include <string.h>