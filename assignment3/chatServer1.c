
#include "inet.h"
#include "structures.h"
#include <ctype.h>
#include <stdlib.h>
#include<signal.h>
#include<pthread.h>

void handle_sigint(int sig);
char* portcopy;
char* namecopy;
SOCKET socket_listen;
int main(int argc, char* argv[]) {

    if (strlen(argv[1]) > 20) {
        printf("Cannot have more than 20 chars in Chatroom Name");
        return 1;
    }

    int len;
    len = strlen(argv[1]);
    namecopy = malloc(len + 1);
    strcpy(namecopy, argv[1]);

    signal(SIGINT, handle_sigint);

    struct chatter* chatters[5] = { NULL };


    int socket_listen;
    struct sockaddr_in my_addr;

    socket_listen = socket(AF_INET, SOCK_STREAM, 0);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(SERV_TCP_PORT);     // short, network byte order
    my_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
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

                    for (int a = 0; a < 5; a++) {
                        printf("%d\n", a);
                        if (chatters[a] == NULL) {
                            chatters[a] = (struct chatter*)malloc(sizeof(struct chatter));
                            chatters[a]->first_send = 0;
                            chatters[a]->socket = socket_client;
                            printf("New connection from %d\n", chatters[a]->socket);
                            break;
                        }
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
                    if (strcmp("DEEEELEEETE", read) == 0) {
                        printf("user has left the chat...\n");
                        remove = 1;

                        for (int a = 0; a < 5; a++) {
                            if (chatters[a] != NULL) {
                                if (chatters[a]->socket == i) {
                                    removechatterindex = a;
                                    strncpy(removedname, chatters[a]->name, sizeof(removedname));
                                }
                            }
                        }
                    }
                    if (remove == 1) {
                        struct chatter* newchatters[5] = { NULL };

                        for (int j = 0; j < 5; j++) {
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
                    for (int k = 0; k < 5; k++) {
                        if (chatters[k] == NULL) {
                            continue;
                        }
                        if (chatters[k]->socket == i) {
                            if (chatters[k]->first_send == 0) {
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
                    for (int k = 0; k < 5; k++) {
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

    return 0;
}


void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);
    CLOSESOCKET(socket_listen);

    printf("Finished.\n");

    exit(0);
}
