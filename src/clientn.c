#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define MAX 1024

void *recv_func(void *socket_desc) {
    int clientfd ;
    memcpy(&clientfd,socket_desc, sizeof(clientfd));
    char rcv[MAX];
    int recr;
    while ((recr = recv(clientfd, rcv, sizeof(rcv) - 1, 0)) > 0) {
        if (recr < 0) {
            perror("Error receiving message");
            break;
        }
        rcv[recr] = '\0';
        printf("%s", rcv);
        memset(rcv, 0, sizeof(rcv));
    }
    if (recr < 0) {
        perror("Receiving failed");
    }
    close(clientfd);
    pthread_exit(NULL);
}

void *send_func(void *socket_desc) {
    int clientfd;
    memcpy(&clientfd,socket_desc, sizeof(clientfd));
    char snd[MAX];
    while (1) {
        if (fgets(snd, sizeof(snd), stdin) == NULL) {
            break;
        }
        if (send(clientfd, snd, strlen(snd), 0) < 0) {
            perror("Send failed");
            break;
        }
        if (strcmp(snd, "exit\n") == 0) {
            printf("EXIT REQUESTED\n");
            break;
        }
        memset(snd, 0, sizeof(snd));
    }
    close(clientfd);
    pthread_exit(NULL);
}

int main(int argc , char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        return 1;
    }

    int clientfd;
    struct sockaddr_in server;
    struct hostent *serverip;

    clientfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    serverip = gethostbyname(argv[1]);
    if (serverip == NULL) {
        fprintf(stderr, "No such host\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    memcpy(&server.sin_addr.s_addr, serverip->h_addr_list[0], serverip->h_length);

    if (connect(clientfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    pthread_t recv_thread, send_thread;

    pthread_create(&recv_thread, NULL, recv_func, &clientfd);
    pthread_create(&send_thread, NULL, send_func, &clientfd);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    return 0;
}
