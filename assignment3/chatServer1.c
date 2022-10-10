
#include "inet.h"
#include "structures.h"
#include <ctype.h>
#include <stdlib.h>
#include<signal.h>
#include<pthread.h>

char** parse_cmd_args(int argc, char *argv[], char ** port_and_name);
void handle_sigint(int sig);
void setup_server(char ** port_and_room);
void connect_new_chatter(struct chatter* chatters[], int socket_client);
int checkduplicatename(char* s, struct chatter* chatters[]);

int socket_listen;
int main(int argc, char* argv[]) {
    int first_chatter = 0;
    struct chatter* chatters[MAX_CHATTERS] = { NULL };
    char **port_and_room = malloc (sizeof (char *) * 3);
    parse_cmd_args(argc, argv, port_and_room);
    signal(SIGINT, handle_sigint);

    setup_server(port_and_room);

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;
    printf("Waiting for connections...\n");

    while (1) {
        fd_set reads;
        reads = master;
        if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
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
                        fprintf(stderr, "accept() failed. (%d)\n",
                            GETSOCKETERRNO());
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

                    connect_new_chatter(chatters, socket_client);

                    
                }
                
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
                        remove = 1;

                        for (int a = 0; a < MAX_CHATTERS; a++) {
                            if (chatters[a] != NULL) {
                                if (chatters[a]->socket == i) {
                                    removechatterindex = a;
                                    strncpy(removedname, chatters[a]->name, sizeof(removedname));
                                    printf("%s has left the chat...\n", removedname);

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
                                sprintf(hmm, "** Welcome to [%s], ", port_and_room[1]);
                                strcat(hmm, chatters[k]->name);
                                strcat(hmm, " **\n");
                                send(chatters[k]->socket, hmm, strlen(hmm), 0);
                                memset(hmm, 0, strlen(hmm));
                                if(first_chatter == 0){
                                    first_chatter++;
                                    strcat(hmm, "You are the first person in this room!\n");
                                    send(chatters[k]->socket, hmm, strlen(hmm), 0);
                                    memset(hmm, 0, strlen(hmm));
                                }
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

    return 0;
}


//////////////////////////////////HELPERS///////////////////////////////////////////////////////////////////

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);
    CLOSESOCKET(socket_listen);

    printf("Finished.\n");

    exit(0);
}


char** parse_cmd_args(int argc, char *argv[], char ** port_and_room){
    char *portcopy;
    char *namecopy;
    if (argc < 3) {
        fprintf(stderr, "Bad command format, try this: ./chatServer1 port \"roomname\"\n");
        exit(1);
    }
    if (argc > 3) {
        fprintf(stderr, "Too many arguments. See Ya!\nDo this next time -> ./chatServer1 port \"roomname\"\n");
        exit(0);
    }

    if (strlen(argv[2]) > 20) {
        printf("Cannot have more than 20 chars in Chatroom Name");
        exit(1);
    }
    printf("\n**ATTENTION** If you entered an invalid port number, I just assign you a good one\n\n");

    int len = strlen(argv[1]);
    portcopy = malloc(len + 1);
    strcpy(portcopy, argv[1]);

    len = strlen(argv[2]);
    namecopy = malloc(len + 1);
    strcpy(namecopy, argv[2]);

    strncpy (port_and_room[0], portcopy, strlen(portcopy)+1);
    strncpy (port_and_room[1], namecopy, strlen(namecopy)+1);
}


void setup_server(char ** port_and_room){
    struct sockaddr_in my_addr;

    socket_listen = socket(AF_INET, SOCK_STREAM, 0);

    short port;
    sscanf(port_and_room[0], "%hi", &port);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);     // short, network byte order
    my_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);

    bind(socket_listen, (struct sockaddr*)&my_addr, sizeof my_addr);

    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        printf("listen() failed.\n");
        exit(1);
    }

    struct sockaddr_in sin;
    socklen_t lendle = sizeof(sin);
    if (getsockname(socket_listen, (struct sockaddr*)&sin, &lendle) == -1)
        perror("getsockname");
    else
        printf("port: %d\n", htons(sin.sin_port));


    char host[15];
    inet_ntop(AF_INET, &(my_addr.sin_addr.s_addr), host, INET_ADDRSTRLEN);
    printf("host: %s\n", host);
}

void connect_new_chatter(struct chatter* chatters[], int socket_client){
    int flag=0;
    for (int a = 0; a <= MAX_CHATTERS; a++) {
        if (chatters[a] == NULL) {
            flag=1;
            printf("%d\n", a);
            chatters[a] = (struct chatter*)malloc(sizeof(struct chatter));
            chatters[a]->first_send = 0;
            chatters[a]->socket = socket_client;
            printf("New connection from %d\n", chatters[a]->socket);
            return;
        }
        else{
            printf("%d\n", a);
            printf("%s\n", chatters[a]->name);
        }

    }
    if (flag ==0){
        char hmm[] = "Sorry max chatters have been reached. See Ya!";
        send(socket_client, hmm, strlen(hmm), 0);
        return;
    }
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