
#include "structures.h"
#include "inet.h"
#include <stdlib.h>
#include<signal.h>


char* selection(int selection, char* text);
void registerclient();
void remove_spaces(char* s);
void handle_sigint(int sig);
void parse_cmd_args(int argc, char *argv[]);
void connect_to_server(char **port_and_name);
void handle_read();
void handle_write();
fd_set setup_select(struct timeval timeout);


char* port;
char* namecopy;
SOCKET sockfd;
int main(int argc, char* argv[]) {
    char **port_and_name = malloc (sizeof (char *) * 3);
    parse_cmd_args(argc, argv);
    signal(SIGINT, handle_sigint);
    registerclient();
    port_and_name[0] = port;
    port_and_name[1] = namecopy;
    connect_to_server(port_and_name);

    
    while (1) {
        struct timeval timeout;
        fd_set reads = setup_select(timeout);
        if (select(sockfd + 1, &reads, 0, 0, &timeout) < 0) {
            fprintf(0, "select() failed\n");
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

    printf("Finished.\n");
    send(sockfd, "\032", 12, 0);
    CLOSESOCKET(sockfd);

    exit(0);
}

void parse_cmd_args(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(0, "./chatClient2 \"username\"\n");
        exit(0);
    }
    if (argc > 2) {
        fprintf(0, "Too many arguments. See Ya!\nDo this next time -> ./chatClient2 \"username\"\n");
        exit(0);
    }

    if (strlen(argv[1]) > 20) {
        printf("Cannot have more than 20 chars in username");
        exit(0);
    }

    int len = strlen(argv[1]);
    namecopy = malloc(len + 1);
    strcpy(namecopy, argv[1]);

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
        printf("Connection closed by peer.\n");
        exit(0);
    }
    if(strcmp("Sorry max chatters have been reached. See Ya!", read) == 0 || strcmp("Someone in the chatroom already has that name. See Ya!", read) == 0) {
        printf("%s\n", read);
        printf("\nClosing socket...\n");
        CLOSESOCKET(sockfd);

        printf("Finished.\n");
        exit(0);
    }

    printf("%s\n", read);
}

void handle_write(SOCKET sockfd){
    char write[100] = {'\0'};
    if (!fgets(write, 100, stdin)) return;
    else if (strcmp("\n", write) ==0) {
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


void registerclient() {
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* peer_address;

    char myport[30];
    sprintf(myport, "%d", SERV_TCP_PORT); 
    if (getaddrinfo(SERV_HOST_ADDR, myport, &hints, &peer_address)) {
        fprintf(0, "getaddrinfo() failed.\n");
        return;
    }

    printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
        address_buffer, sizeof(address_buffer),
        service_buffer, sizeof(service_buffer),
        NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);


    printf("Creating socket...\n");
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
        peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        fprintf(0, "socket() failed.\n");
        return;
    }


    printf("Connecting...\n");
    if (connect(socket_peer,
        peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(0, "connect() failed.\n");
        return;
    }
    freeaddrinfo(peer_address);
    send(socket_peer, namecopy, 7, 0);
    printf("Connected.\n");
    printf("To send data, enter text followed by enter.\n");

    while (1) {

        fd_set reads;
        fd_set writes;
        FD_ZERO(&reads);
        FD_SET(0, &reads);
        FD_SET(socket_peer, &reads);


        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0) {
            fprintf(0, "select() failed.n");
            return;
        }

        if (FD_ISSET(socket_peer, &reads)) {
            char read[1024] = { '\0' };
            int bytes_received = recv(socket_peer, read, 1024, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            int i = 0;
            for (i = 0; read[i] != '\0'; ++i);

            printf("\n   %s\n", read);

            int a;
            char write[20];
            scanf("%d", &a);
            port = selection(a, read);
            if (port == NULL) {
                printf("Invalid selection, returned NULL");
                close(socket_peer);
                return;
            }
            remove_spaces(port);
            
        }
        close(socket_peer);
        return;
    } //end while(1)
}

void remove_spaces(char* s) {
    char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
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
