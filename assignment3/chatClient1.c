
#include "structures.h"
#include "inet.h"
#include <stdlib.h>
#include<signal.h>

void registerclient();
void handle_sigint(int sig);
void parse_cmd_args(int argc, char *argv[], char ** port_and_name);
void connect_to_server(char **port_and_name);
void handle_read();
void handle_write();
fd_set setup_select(struct timeval timeout);


SOCKET sockfd;
int main(int argc, char* argv[]) {
    char **port_and_name = malloc (sizeof (char *) * 3);
    parse_cmd_args(argc, argv, port_and_name);

    


    signal(SIGINT, handle_sigint);
    connect_to_server(port_and_name);
    
    while (1) {
        struct timeval timeout;
        fd_set reads = setup_select(timeout);
        if (select(sockfd + 1, &reads, 0, 0, &timeout) < 0) {
            printf("select() failed.\n");
            return 1;
        }
        if (FD_ISSET(sockfd, &reads)) {
            handle_read(sockfd);
        }
        if (FD_ISSET(0, &reads)) {
            handle_write(sockfd);
        }
    } //end while(1)

    printf("Closing socket...\n");
    CLOSESOCKET(sockfd);
    printf("Finished.\n");
    return 0;
}


void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);

    printf("Goodbye.\n");
    send(sockfd, "\032", 12, 0);
    CLOSESOCKET(sockfd);

    exit(0);
}


void parse_cmd_args(int argc, char *argv[], char ** port_and_name){
    char* portcopy;
    char* namecopy;
    if (argc < 3) {
        printf("Not enough Arguments.\nDo this next time -> ./chatClient1 serverport \"username\"\nGoodbye.\n");
        exit(0);
    }
    if (argc > 3) {
        printf("Too many arguments.\nDo this next time -> ./chatClient1 serverport \"username\"\nGoodbye.\n");
        exit(0);
    }

    if (strlen(argv[2]) > 20) {
        printf("Cannot have more than 20 chars in Chatroom Name");
        exit(0);
    }

    port_and_name[0] = malloc(strlen(argv[1])+1);
    port_and_name[1] = malloc(strlen(argv[2])+1);
    strcpy(port_and_name[0], argv[1]);
    strcpy(port_and_name[1], argv[2]);
}


void connect_to_server(char **port_and_name){
    struct sockaddr_in  serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    unsigned short port;
    sscanf(port_and_name[0], "%hu", &port);
    printf("port %hu\n", port);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);     // short, network byte order
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client: can't open stream socket. Goodbye.\n");
        exit(1);
    }

    char host[15];
    inet_ntop(AF_INET, &(serv_addr.sin_addr.s_addr), host, INET_ADDRSTRLEN);
    /* Connect to the server. */
    if (connect(sockfd, (struct sockaddr*)&serv_addr,
        sizeof(serv_addr)) < 0) {
        perror("client: can't connect to server. Goodbye.\n");
        exit(1);
    }

    send(sockfd, port_and_name[1], strlen(port_and_name[1]), 0);
    printf("Connected.\n\n");
    printf("                    @@ ATTENTION @@\n*********************************************************");
    printf("\nCannot write more than 99 characters. If you submit > 99,\nI will only send the first 99 chars to the other chatters\n");
    printf("*********************************************************\n\n");
}

void handle_read(SOCKET sockfd){
    char read[1024] = { '\0' };
    int bytes_received = recv(sockfd, read, 1024, 0);
    if (bytes_received < 1) {
        printf("Connection closed by peer. Goodbye.\n");
        exit(0);
    }
    if(strcmp("Sorry max chatters have been reached. See Ya!", read) == 0 || strcmp("Someone in the chatroom already has that name. See Ya!", read) == 0) {
        printf("%s\n", read);
        printf("\nClosing socket...\n");
        CLOSESOCKET(sockfd);

        printf("Goodbye..\n");
        exit(0);
    }

    printf("%s\n", read);
}

void handle_write(SOCKET sockfd){
    char write[100] = {'\0'};
    if (!fgets(write, 100, stdin)) return;
    else if (strcmp("\n", write) ==0) {
        printf("<Cant send nothing to other chatters!>\n");
        return;
    }
    send(sockfd, write, strlen(write), 0);
    printf("------You sent that------\n\n");
}


fd_set setup_select(struct timeval timeout){
    fd_set reads;
    FD_ZERO(&reads);
    FD_SET(0, &reads);
    FD_SET(sockfd, &reads);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    return reads;
}

void checkforspaces(char *name){
    if (strchr(name, ' ') != NULL)
    {
        printf("No spaces allowed in username. Goodbye.\n");
        exit(0);
    }
}
