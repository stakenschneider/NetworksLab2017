//Client trade

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

void SentErr (char *s)
{
    perror(s);
    exit(1);
}

void *SendHandler(void* socket) {
    int s = (int) socket;
    int rc;
    while (1) {
    char *text;
            text =(char*) malloc (sizeof(char));
            if (text == NULL)
                 SentErr("Can't malloc");
            //Getting text from keyboard
            gets(text);
	    rc = send( s, text, 20, 0 );
	    if ( rc <= 0 )
                 SentErr("Sent call error");
    }

}

int main( void )
{
    struct sockaddr_in peer;
    int s;
    int rc;
    char buf[ BUF_SIZE ];
    //Fill sockaddr_in
    peer.sin_family = AF_INET;
    peer.sin_port = htons( PORT );
    peer.sin_addr.s_addr = inet_addr( SRV_IP );
    //we set up sockaddr_in
    if (inet_aton(SRV_IP, &peer.sin_addr)==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit (1);
    }
    //Get socket
    s = socket( AF_INET, SOCK_STREAM, 0 );
    if ( s < 0 )
        SentErr("Socket call failed");
    //Making connection
    rc = connect( s, ( struct sockaddr * )&peer, sizeof( peer ) );
    if ( rc )
        SentErr("Connect call failed");

    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    pthread_t send_thread;
    //Working with forum

    rc = pthread_create(&send_thread, &threadAttr, SendHandler, (void*) s);
    while (1)
    {
        rc = recv( s, buf, BUF_SIZE, 0 );
	if ( rc <= 0 )
	    SentErr("Recive call failed");
	else
        {
            int i=0;
            while (buf[i]!=NULL)
            {
                if (buf[0]=='^')//If we doing something wrong
                {
                    printf("Invalid choose.Press enter and Try again\n");
                }
                if (buf[0]=='#')//If we was disconected
                {
                    printf("Closing connection...\n");
                    return 0;
                }
                printf("%c",buf[i]);
                i++;
            }
             printf("\n____________________\n");
        }
    }
    return 0;
}


