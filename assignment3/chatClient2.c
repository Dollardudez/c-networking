
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
char* namecopy;
SOCKET sockfd;
int main(int argc, char* argv[]) {

    
    int len = strlen(argv[1]);
    namecopy = malloc(len + 1);
    strcpy(namecopy, argv[1]);

    if (argc < 1) {
        fprintf(stderr, "./chatClient \"Username\"\n");
        return 1;
    }
    if (argc > 2) {
        fprintf(stderr, "Too many arguments. See Ya!\nDo this next time -> ./chatClient \"Username\"\n");
        exit(0);
    }

    registerclient();

    printf("Configuring remote address...\n");
    signal(SIGINT, handle_sigint);

    struct sockaddr_in  serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    serv_addr.sin_port = htons(atoiport);

    printf("htons    %d\n", atoiport);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client: can't open stream socket");
        exit(1);
    }

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

void registerclient() {
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* peer_address;
    if (getaddrinfo(SERV_HOST_ADDR, "8080", &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
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
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }


    printf("Connecting...\n");
    if (connect(socket_peer,
        peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
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
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
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
            printf("da port = \n\n\n%s\n", port);
            atoiport =  atoi(port);
            
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

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);

    printf("Finished.\n");
    send(sockfd, "DEEEELEEETE", 12, 0);
    CLOSESOCKET(sockfd);

    exit(0);
}

