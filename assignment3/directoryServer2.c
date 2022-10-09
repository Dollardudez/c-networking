
#include "structures.h"
#include "inet.h"
#include <ctype.h>
#include <stdlib.h>

char* getServerText(struct chatroom* rooms[]);
int registerchatroom(char* s, struct chatroom** rooms);
int checkforchatroom(char s[]);
int checkduplicatename(char* s, struct chatroom** rooms);



int main(int argc, char* argv[]) {

    struct chatroom* chatrooms[5] = { NULL };



    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* bind_address;
    getaddrinfo(0, "8080", &hints, &bind_address);


    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
        bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Binding socket to local address...\n");
    if (bind(socket_listen,
        bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);


    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

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
                    
                    printf("New connection from %d\n", i);


                }
                ////////////////////////////////////////////////////////////////////////////////////////////////////
                else {
                    char read[1024];
                    int bytes_received = recv(i, read, 1024, 0);
                    if (bytes_received < 1) {
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }
                    int result = 10;
                    int isChatroom = checkforchatroom(read);
                    if (isChatroom == 0) {
                        char* data = getServerText(chatrooms);
                        send(i, data, strlen(data), 0);
                    }
                    else {
                        result = registerchatroom(read, chatrooms);
                    }
                    if (result == 0) {
                        struct chatroom* newchatrooms[5] = { NULL };

                        for (int i = 0; i < 5; i++) {
                            if (chatrooms[i] != NULL) {
                                if (chatrooms[i]->active != 0) {
                                    
                                 newchatrooms[i] = chatrooms[i];

                                }
                            }                        
                        }
                        memcpy(chatrooms, newchatrooms, sizeof(newchatrooms));
                        //free(newchatrooms);
                    }

                    if (result == 2) {
                        send(i, "Chatroom name already exists", 29, 0);
                    }
                }

            } //if FD_ISSET
        } //for i to max_socket116
    } //while(1)



    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);

    printf("Finished.\n");

    return 0;
}

char* getServerText(struct chatroom* rooms[]) {
    char* roominf = malloc(1024);
    strncpy(roominf, "List of active chatrooms\n\n", 27);
    for (int i = 0; i < 5; i++) {
        if (rooms[i] == NULL) continue;
        else{
            char buffer[100];
            sprintf(buffer, "%d ) Name: %s | Socket: %d | Active: %d\n\n", i, rooms[i]->name, rooms[i]->socket, rooms[i]->active);
            strcat(roominf, buffer);
        }
    }
    char* returnedstring = malloc(1024);
    strncpy(returnedstring, roominf, strlen(roominf));
    return returnedstring;
}

int registerchatroom(char * s, struct chatroom** rooms){
    
    char tokens[3][100];
    char* token = strtok(s, " ");
    int i = 0;
    while (token != NULL)
    {
        i++;
        printf("%s\n", token);
        if (i == 1) {
            //tokens[0] = malloc(strlen(token));
            strncpy(tokens[0], token, strlen(token));
        }
        if (i == 2) {
            //tokens[1] = malloc(strlen(token));
            strncpy(tokens[1], token, strlen(token));
        }
        if (i == 3) {
            //tokens[2] = malloc(strlen(token));
            strncpy(tokens[2], token, strlen(token));
        }
        token = strtok(NULL, " ");
    }
    
    if (strcmp(tokens[2], "0") == 0) {
        for (int a = 0; a < 5; a++) {
            if (strcmp(rooms[a]->name, tokens[0]) == 0) {
                char* ptr;
                rooms[a]->active = strtol(tokens[2], &ptr, 10);
                break;
            }
        }
        return 0;
    }
    else {
        if (checkduplicatename(tokens[0], rooms) == 0) return 2;
        for (int a = 0; a < 5; a++) {
            if (rooms[a] == NULL) {
                rooms[a] = (struct chatroom*)malloc(sizeof(struct chatroom));
                strncpy(rooms[a]->name, tokens[0], sizeof(rooms[a]->name));
                char* ptr;
                rooms[a]->socket = strtol(tokens[1], &ptr, 10);
                char* otherptr;
                rooms[a]->active = strtol(tokens[2], &otherptr, 10);
                break;
            }
        }
    }
    return 1;
}


int checkforchatroom(char s[]) {
    for (int i = 0; s[i] != '\0'; ++i) {
        if (s[i] == ' ') {
            
            return 1;
        }
    }
    return 0;
        
}

int checkduplicatename(char* s, struct chatroom** rooms) {
    for (int i = 0; i < 5; i++) {
        if (rooms[i] != NULL) {
            if (strcmp(rooms[i]->name, s) == 0) {
                return 0;

            }
        }
    }
    return 1;
}