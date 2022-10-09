
#include "structures.h"
#include "inet.h"
#include <stdlib.h>
#include<signal.h>

char* selection(int selection, char* text);
void registerclient();
void remove_spaces(char* s);
void handle_sigint(int sig);

char* port;
int atoiport;
char* portcopy;
char* namecopy;
SOCKET sockfd;
int main(int argc, char* argv[]) {

    if (argc < 3) {
        fprintf(stderr, "./chatClient1 serverport \"username\"\n");
        return 1;
    }
    if (argc > 3) {
        fprintf(stderr, "Too many arguments. See Ya!\nDo this next time -> ./chatClient1 serverport \"username\"\n");
        exit(0);
    }

    if (strlen(argv[2]) > 20) {
        printf("Cannot have more than 20 chars in Chatroom Name");
        return 1;
    }


    int len = strlen(argv[1]);
    portcopy = malloc(len + 1);
    strcpy(portcopy, argv[1]);

    len = strlen(argv[2]);
    namecopy = malloc(len + 1);
    strcpy(namecopy, argv[2]);


    printf("Configuring remote address...\n");
    signal(SIGINT, handle_sigint);

    struct sockaddr_in  serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    short port;
    sscanf(portcopy, "%hi", &port);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);     // short, network byte order
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);

    printf("htons -> %d\n", atoiport);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client: can't open stream socket");
        exit(1);
    }

    char host[15];
    inet_ntop(AF_INET, &(serv_addr.sin_addr.s_addr), host, INET_ADDRSTRLEN);
    printf("Attempting connection to-> %s:%d\n", host, htons(serv_addr.sin_port));
    /* Connect to the server. */
    if (connect(sockfd, (struct sockaddr*)&serv_addr,
        sizeof(serv_addr)) < 0) {
        perror("client: can't connect to server");
        exit(1);
    }

    send(sockfd, namecopy, strlen(namecopy), 0);
    printf("Connected.\n\n");
    while (1) {

        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(0, &reads);
        FD_SET(sockfd, &reads);


        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(sockfd + 1, &reads, 0, 0, &timeout) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        if (FD_ISSET(sockfd, &reads)) {
            char read[1024] = { '\0' };
            int bytes_received = recv(sockfd, read, 1024, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }

            printf("%s\n", read);
           

        }
        
        if (FD_ISSET(0, &reads)) {
            char write[1024];
            if (!fgets(write, 1024, stdin)) break;
            if (strlen(write) < 1) {
                printf("Cant send nothing to other chatters!");
                break;
            }
            send(sockfd, write, strlen(write), 0);
        }
    } //end while(1)

    printf("Closing socket...\n");
    CLOSESOCKET(sockfd);

    printf("Finished.\n");
    return 0;
}

char* selection(int selection, char* text) {
    
    char* token = strtok(text, "\n");
    token = strtok(NULL, "\n");
    int i = 0;
    while (token != NULL)
    {
        printf("%s\n", token);
        if (i == selection) {
            char* tokenception = strtok(token, " ");
            int j;
            while (tokenception != NULL)
            {
                if (j == 7) {
                    return tokenception;
                }
                tokenception = strtok(NULL, " ");
                j++;
            }
        }
        
        token = strtok(NULL, "\n");
        i++;
    }
    return NULL;
}

void remove_spaces(char* s) {
    char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);

    printf("Finished.\n");
    send(sockfd, "DEEEELEEETE", 12, 0);
    CLOSESOCKET(sockfd);

    exit(0);
}

