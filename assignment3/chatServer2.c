
#include "inet.h"
#include "structures.h"
#include <ctype.h>
#include <stdlib.h>
#include<signal.h>
#include<pthread.h>

void parse_cmdline_args(int argc, char *argv[]);
void registerwithdir(char host[], char port[], char name[], int cmd);
void handle_sigint(int sig);
int checkduplicatename(char* s, struct chatter* chatters[]);
int IsValidNumber(char * string);

char* portcopy;
char* namecopy;
char* hostcopy;

SOCKET socket_listen;
int main(int argc, char* argv[]) {
    int first_chatter = 0;
    
    parse_cmdline_args(argc, argv);
    signal(SIGINT, handle_sigint);

    struct chatter* chatters[MAX_CHATTERS] = { NULL };

    int socket_listen;
    struct sockaddr_in my_addr;

    socket_listen = socket(AF_INET, SOCK_STREAM, 0);
    short port;
    sscanf(portcopy, "%hi", &port);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);     // short, network byte order
    my_addr.sin_addr.s_addr = inet_addr(INADDR_ANY);
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);

    bind(socket_listen, (struct sockaddr*)&my_addr, sizeof my_addr);


    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        printf("listen() failed.\n");
        return 1;
    }

    struct sockaddr_in sin;
    socklen_t lendle = sizeof(sin);
    if (getsockname(socket_listen, (struct sockaddr*)&sin, &lendle) == -1)
        perror("getsockname");
    else
        printf("port number: %d\n", htons(sin.sin_port));

    char text[10];
    sprintf(text, "%d", htons(sin.sin_port));
    hostcopy = inet_ntoa(my_addr.sin_addr.s_addr);
    printf("%s", host);


    registerwithdir(host, text, argv[2], 1);



    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    printf("Waiting for connections...\n");


    while (1) {
        fd_set reads;
        reads = master;
        if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
            printf("select() failed.\n");
            return 1;
        }

        SOCKET i;
        for (i = 1; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {

                if (i == socket_listen) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(socket_listen,
                        (struct sockaddr*)&client_address,
                        &client_len);
                    if (!ISVALIDSOCKET(socket_client)) {
                        printf("accept() failed.\n");
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;

                    char address_buffer[1000];
                    getnameinfo((struct sockaddr*)&client_address,
                        client_len,
                        address_buffer, sizeof(address_buffer), 0, 0,
                        NI_NUMERICHOST);

                    int flag=0;

                    for (int a = 0; a < MAX_CHATTERS; a++) {
                        printf("%d\n", a);
                        if (chatters[a] == NULL) {
                            flag = 1;
                            chatters[a] = (struct chatter*)malloc(sizeof(struct chatter));
                            chatters[a]->first_send = 0;
                            chatters[a]->socket = socket_client;
                            printf("New connection from %d\n", chatters[a]->socket);
                            break;
                        }
                    
                    }
                    if(flag == 0){
                        char hmm[] = "Sorry max chatters have been reached. See Ya!";
                        send(socket_client, hmm, strlen(hmm), 0);
                        break;
                    }
                    
                }
                ////////////////////////////////////////////////////////////////////////////////////////////////////
                else {
                    int check = 0;
                    char full_message[1024] = { '\0' };
                    char sendername[21] = { '\0' };
                    char read[1024] = { '\0' };
                    int bytes_received = recv(i, read, 1024, 0);
                    if (bytes_received < 1) {
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }
                    int removechatterindex = 100;
                    int remove = 0;
                    char removedname[21];
                    if (strcmp("\032", read) == 0) {
                        printf("A user has left the chat...\n");
                        remove = 1;

                        for (int a = 0; a < MAX_CHATTERS; a++) {
                            if (chatters[a] != NULL) {
                                if (chatters[a]->socket == i) {
                                    removechatterindex = a;
                                    strncpy(removedname, chatters[a]->name, sizeof(removedname));
                                }
                            }
                        }
                    }
                    if (remove == 1) {
                        struct chatter* newchatters[MAX_CHATTERS] = { NULL };

                        for (int j = 0; j < MAX_CHATTERS; j++) {
                            if (j != removechatterindex) {
                                if (chatters[j] != NULL) {
                                    newchatters[j] = chatters[j];
                                }
                            }

                        }
                        memcpy(chatters, newchatters, sizeof(newchatters));
                        check = 3;
                    }
                    memset(full_message, 0, strlen(full_message));
                    memset(sendername, 0, strlen(sendername));
                    for (int k = 0; k < MAX_CHATTERS; k++) {
                        if (chatters[k] == NULL) {
                            continue;
                        }
                        if (chatters[k]->socket == i) {
                            if (chatters[k]->first_send == 0) {
                                if (checkduplicatename(read, chatters)==0){
                                    char hmm[] = "Someone in the chatroom already has that name. See Ya!";
                                    send(chatters[k]->socket, hmm, strlen(hmm), 0);
                                    chatters[k] = NULL;
                                    break;
                                }
                                strncpy(chatters[k]->name, read, sizeof(chatters[k]->name));


                                chatters[k]->first_send = 1;
                                check = 1;
                            }
                            else {

                                check = 2;
                            }
                            strncpy(sendername, chatters[k]->name, sizeof(sendername));
                        }
                    }
                    for (int k = 0; k < MAX_CHATTERS; k++) {
                        strcat(full_message, sendername);
                        if (chatters[k] == NULL) {
                            memset(full_message, 0, strlen(full_message));
                            continue;
                        }
                        if (check == 1) {
                            if (chatters[k]->socket == i) {
                                char hmm[100];
                                sprintf(hmm, "** Welcome to [%s], ", namecopy);
                                strcat(hmm, chatters[k]->name);
                                strcat(hmm, " **\n");
                                if(first_chatter == 0){
                                    strcat(hmm, "You are the first person in the chatroom.\n");
                                    first_chatter = 1;
                                }
                                send(chatters[k]->socket, hmm, strlen(hmm), 0);
                                memset(hmm, 0, strlen(hmm));
                                memset(full_message, 0, strlen(full_message));

                            }
                            else {
                                char hmm[] = " has joined the chat.\n";
                                strcat(full_message, hmm);
                                send(chatters[k]->socket, full_message, strlen(full_message), 0);
                                memset(full_message, 0, strlen(full_message));
                                memset(hmm, 0, strlen(hmm));
                            }
                        }
                        if (check == 2) {
                            if (chatters[k]->socket == i) {
                                memset(full_message, 0, strlen(full_message));
                                continue;
                            }
                            else {
                                char hmm[] = ": ";
                                strcat(full_message, hmm);
                                strcat(full_message, read);
                                send(chatters[k]->socket, full_message, strlen(full_message), 0);
                                memset(full_message, 0, strlen(full_message));
                                memset(hmm, 0, strlen(hmm));

                            }
                        }
                        if (check == 3) {
                            char hmm[] = " has left the chat :(";
                            strcat(removedname, hmm);
                            send(chatters[k]->socket, removedname, strlen(removedname), 0);
                            memset(removedname, 0, strlen(removedname));
                            memset(full_message, 0, strlen(full_message));
                            memset(hmm, 0, strlen(hmm));
                        }

                    }
                }

            } //if FD_ISSET
        } //for i to max_socket
    } //while(1)




    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);

    registerwithdir(hostcopy, argv[1], argv[2], 0);
    return 0;
}

void registerwithdir(char port[], char name[], int cmd) {

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* peer_address;
    char myport[30];
    sprintf(myport, "%d", SERV_TCP_PORT); 
    if (getaddrinfo(SERV_HOST_ADDR, myport, &hints, &peer_address)) {
        printf("getaddrinfo() failed.\n");
        return;
    }
    int netport = htons(atoi(port));

    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
        address_buffer, sizeof(address_buffer),
        service_buffer, sizeof(service_buffer),
        NI_NUMERICHOST);


    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
        peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        printf("socket() failed.\n");
        return;
    }


    if (connect(socket_peer,
        peer_address->ai_addr, peer_address->ai_addrlen)) {
        printf("connect() failed.\n");
        return;
    }
    freeaddrinfo(peer_address);
    char strData[300];
    strncpy(strData, name, sizeof(strData) + 1);
    strcat(strData, "^^^^");
    strcat(strData, hostcopy);
    strcat(strData, "^^^^");
    strcat(strData, port);
    strcat(strData, "^^^^");
    char str[2];
    sprintf(str, "%d", cmd);
    strcat(strData, str);
    send(socket_peer, strData, sizeof(strData) + 1, 0);

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
            printf("select() failed.\n");
            return;
        }

        if (FD_ISSET(socket_peer, &reads)) {
            char read[1024] = { '\0' };
            int bytes_received = recv(socket_peer, read, 1024, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                exit(0);
            }
            int i = 0;
            for (i = 0; read[i] != '\0'; ++i);

            if (strcmp(read, "Chatroom name already exists") == 0 || strcmp(read,"A max of 3 active chatrooms has been reached.") == 0) {
                printf("\n\n** %s **\n", read);
                printf("\n** Goodbye **\n");
                exit(0);
            }
            fflush(stdout);
        }


        if (FD_ISSET(0, &reads)) {
            char write[1024];
            if (!fgets(write, 1024, stdin)) break;
            int bytes_sent = send(socket_peer, write, strlen(write), 0);
        }
        printf("Registered with Directory Server\n");
        close(socket_peer);
        return;
    } //end while(1)
}

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);
    CLOSESOCKET(socket_listen);

    registerwithdir(hostcopy, portcopy, namecopy, 0);
    printf("Finished.\n");

    exit(0);
}

int checkduplicatename(char* s, struct chatter* chatters[]) {
    for (int i = 0; i < MAX_CHATTERS; i++) {
        if (chatters[i] != NULL) {
            if (strcmp(chatters[i]->name, s) == 0) {
                return 0;

            }
        }
    }
    return 1;
}

void parse_cmdline_args(int argc, char *argv[]){
    if (argc < 3) {
        printf("./chatServer port \"roomname\"\n");
        exit(1);
    }
    if (argc > 3) {
        printf("Too many arguments. See Ya!\nDo this next time -> ./chatServer1 port \"roomname\"\n");
        exit(0);
    }

    if (strlen(argv[2]) > 20) {
        printf("Cannot have more than 20 chars in Chatroom Name");
        exit(1);
    }

    if (IsValidNumber(argv[1])) {
        printf("port must be a number.");
        exit(1);
    }


    printf("If you entered an invalid port number, I will just assign you a good one\n");

    int len;
    len = strlen(argv[1]);
    portcopy = malloc(len + 1);
    strcpy(portcopy, argv[1]);

    len = strlen(argv[2]);
    namecopy = malloc(len + 1);
    strcpy(namecopy, argv[2]);
}

int IsValidNumber(char * string)
{
   for(int i = 0; i < strlen( string ); i ++)
   {
      //ASCII value of 0 = 48, 9 = 57. So if value is outside of numeric range then fail
      //Checking for negative sign "-" could be added: ASCII value 45.
      if (string[i] < 48 || string[i] > 57)
         return 0;
   }

   return 1;
}