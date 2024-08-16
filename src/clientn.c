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
    int clientfd = *(int *)socket_desc;
    char rcv[MAX];
    int recr;
    while ((recr = recv(clientfd, rcv, sizeof(rcv) - 1, 0)) > 0) {
        rcv[recr] = '\0';
        printf("%s", rcv);
        memset(rcv, 0, sizeof(rcv));
    }
    if(recr < 0){
	    perror("receiving failed\n");
    }
    /*
    Closing the clientfd and thread
    */
    close(clientfd);
    pthread_exit(NULL);
}

void *send_func(void *socket_desc) {
    int clientfd = *(int *)socket_desc;
    char snd[MAX];
    while (1) {
        if (fgets(snd, sizeof(snd), stdin) == NULL) {
            break;
        }
        if (send(clientfd, snd, strlen(snd), 0) < 0) {
            perror("send failed");
            break;
        }
        if (strcmp(snd, "exit\n") == 0) {
            printf("EXIT REQUESTED\n");
            break;
        }
	/*
	clearing the send buffer
    */
	memset(snd, 0, sizeof(snd));
    }
    close(clientfd);
    pthread_exit(NULL);
}

int main(int argc , char *argv[]) {
    int clientfd;
    struct sockaddr_in server;
    struct hostent *serverip;
    /*
    create a socket
    */
    clientfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    serverip = gethostbyname(argv[1]);
    if (serverip == NULL) {
        printf("No such host\n");
        return -1;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
//    server.sin_addr.s_addr = inet_addr("127.0.0.1");
     memcpy((char *)&server.sin_addr.s_addr,serverip->h_addr_list[0],serverip->h_length);
    // Connect to server
    if (connect(clientfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed");
        return 1;
    }

    pthread_t recv_thread, send_thread;

    // Create threads for receiving and sending messages
    pthread_create(&recv_thread, NULL, recv_func, &clientfd);
    pthread_create(&send_thread, NULL, send_func, &clientfd);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    return 0;
}
