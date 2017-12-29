#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define SRV_IP "127.0.0.1"
#define PORT 5001
#define BUF_SIZE 1000

void SentErr(char *s) {
    perror(s);
    exit(1);
}

void *SendHandler(void *socket) {
    int s = (int) socket;
    int rc;
    while (1) {
        char text[BUF_SIZE] = "";
        fgets(text, BUF_SIZE, stdin);
        rc = send(s, text, BUF_SIZE, 0);
        if (rc <= 0)
            SentErr("sent call error");
    }
}


int main(void) {
    struct sockaddr_in peer;
    int s;
    int rc;
    char buf[BUF_SIZE];

    peer.sin_family = AF_INET;
    peer.sin_port = htons(PORT);
    peer.sin_addr.s_addr = inet_addr(SRV_IP);

    if (inet_aton(SRV_IP, &peer.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        SentErr("socket call failed");

    rc = connect(s, (struct sockaddr *) &peer, sizeof(peer));
    if (rc)
        SentErr("connect call failed");

    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    pthread_t send_thread;

    rc = pthread_create(&send_thread, &threadAttr, SendHandler, (void *) s);
    while (1) {
        rc = recv(s, buf, BUF_SIZE, 0);
        if (rc <= 0)
            SentErr("receive call failed");
        else {
            int i = 0;
            while (buf[i] != NULL) {
                if (buf[0] == '^')
                    printf("invalid choose\n");


                if (buf[0] == '#') {
                    printf("closing connection\n");
                    return 0;
                }

                printf("%c", buf[i]);
                i++;
            }
            printf("\n");
        }
    }
}