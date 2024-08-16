/*
Author : samip regmi
file:server.c
Aug 14,2024
A simple TCP server for communicating with clients
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX 1024
#define MAX_CLIENTS 100

int clients[MAX_CLIENTS];
int client_count = 0;

void *client_handler(void *clientFD) {
    int clientfd ;
    memcpy(&clientfd,clientFD,sizeof(clientfd));
    char buffer[MAX];
    int recvd;

    /*
   client joining message to all other client except the client which joined
   i dont know why the fuck i have to add 20 byte for using snprintf but yep it works and i
   dont know why
    */


    char join_message[MAX + 20];
    snprintf(join_message, sizeof(join_message), "client[%d]: joined the server\n", clientfd);
    for (int i = 0; i < client_count; i++) {
    if(clients[i]!=clientfd){
        send(clients[i], join_message, strlen(join_message),0);
    }
    }
    printf("client count :%d\n",client_count);
    /*
    Receiving the message from client and storing it in buffer
    Char buffer[]: defined in line 20
    recvd takes total bytes it received .
    making last byte as Null terminator
    */
    while ((recvd = recv(clientfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[recvd] = '\0';
        for (int i = 0; i < client_count; i++) {
            /*
            sending message to all clients except the client which sent the message
            */
            if (clients[i] != clientfd) {
                char message[MAX+20];
                snprintf(message, sizeof(message), "client[%d]: %s", clientfd, buffer);
                send(clients[i], message, strlen(message), 0);
            }
            if (clients[i] == clientfd) {
                char message[MAX+40];
                snprintf(message, sizeof(message), "[+]::You wrote as client[%d]: %s", clientfd, buffer);
                send(clients[i], message, strlen(message), 0);
            }
            /*
           sending the exit message to other clients
            */
            if (strcmp(buffer, "exit\n") == 0) {
                char exit[MAX+20];
                snprintf(exit, sizeof(exit), "!!->client[%d] EXITED THE CHATROOM\n", clientfd);
                send(clients[i],exit,  strlen(exit), 0);
                // break;
            }

        }
        /*
       If exit message received , remove the client and remove the fd of client
       also decrease the client count
        */
        if (strcmp(buffer, "exit\n") == 0) {
            printf("Client %d requested to exit.\n", clientfd);
            close(clientfd);
            for (int i = 0; i < client_count; i++) {
                if (clients[i] == clientfd) {
                    clients[i] = clients[client_count-=1];
                    break;
                }
            }
            pthread_exit(NULL);
            break;
        }

        if (strcmp(buffer, "help-info\n") == 0) {
            char *help = "*******\nFROM SERVER [help-h]\nThis is ChatroomC\nMade by samip regmi\n";
            send(clientfd, help,strlen(help), 0 );
        }
        if (strcmp(buffer, "help-cc\n") == 0) {
            char clientinfo[MAX+40];
            snprintf(clientinfo, sizeof(clientinfo),"*******\nFROM SERVER [help-cc]\nClients active : %d\n",client_count );
            send(clientfd, clientinfo,strlen(clientinfo), 0 );
        }
        //clearing the buffer
        memset(buffer, 0, sizeof(buffer));

    }

    // pthread_exit(NULL);
}

int main() {
    int serverfd, clientfd;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);
    pthread_t thread;

    // Create socket
    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(serverfd, (struct sockaddr *)&server, sizeof(server));
    listen(serverfd, MAX_CLIENTS);

    printf("Server listening on port %d\n", PORT);
    /*
   accepting the clients and creating a thread for each client
    */
    while ((clientfd = accept(serverfd, (struct sockaddr *)&client, &client_len)) >= 0) {
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = clientfd;
            if (pthread_create(&thread, NULL, client_handler, &clientfd) != 0) {
                perror("Could not create thread");
                close(clientfd);
            }
            pthread_detach(thread);
        } else {
            printf("Max clients reached. Connection refused: %d\n", clientfd);
            close(clientfd);
        }
    }

    close(serverfd);
    return 0;
}
