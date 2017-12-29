#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 5001
#define BUF_SIZE 1000
#define TITLES "topics.txt"
#define REC "Receive call failed"

void *ClientHandler(void *socket);
void *ServerHandler();

void SendErrorToClient(int socket);
void SentErrServer(char *s);
int DeleteClient(int number);
int *FindBySocket(int socket);

void add_news(char *topic, char *caption, char *text);
void list_topics();
int list_caption(char buf[], int socket);
int show_text(char buf[], int socket);
int amt_caption(char cap[]);
void to_client(int socket, char *message);

int s, threads = -1;
char topics[BUF_SIZE], caption[BUF_SIZE], chch[BUF_SIZE];
char kill_command[] = "kill", shutdown_command[] = "shutdown";

struct clients {
    int s1;
} *users;


int main(void) {

    users = (char *) malloc(sizeof(char));
    if (users == NULL)
        exit(1);

    printf("server trade is working\n");

    struct sockaddr_in local, si_other;
    int s1, rc, slen = sizeof(si_other);

    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        SentErrServer("socket call failed");

    rc = bind(s, (struct sockaddr *) &local, sizeof(local));
    if (rc < 0)
        SentErrServer("bind call failure");

    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    pthread_t server_thread;
    rc = pthread_create(&server_thread, &threadAttr, ServerHandler, (void *) NULL);

    rc = listen(s, 5);
    if (rc)
        SentErrServer("listen call failed");
    list_topics();

    while (1) {

        s1 = accept(s, (struct sockaddr *) &si_other, &slen);

        if (s1 < 0)
            SentErrServer("accept call failed");

        users[threads + 1].s1 = s1;

        pthread_t client_thread;

        rc = pthread_create(&client_thread, &threadAttr, ClientHandler, (void *) s1);

        if (rc != 0)
            SentErrServer("creating thread false");
        threads++;
    }
}


void *ServerHandler() {
    char text[40];

    while (1) {

        fgets(text, BUF_SIZE, stdin);

        if (strstr(kill_command, text) != NULL) {

            if (DeleteClient((int)text) == 0)
                printf("%s was killed\n", text);
            else
                printf("wrong command. Please try again\n");
        }
    }
}


void to_client(int socket, char *message) {
    int rc;
    rc = send(socket, message, BUF_SIZE, 0);
    if (rc <= 0)
        perror("send call failed");
}


void *ClientHandler(void *socket) {
    int rc;
    char buf[BUF_SIZE];
    char pick = '0';

    while (1) {
        switch (pick) {
            case '0': {
                to_client((int) socket, "\n_______٩(ఠ益ఠ)۶_______\n"
                        "1. view list of topics\n"
                        "2. add news\n"
                        "3. exit\n"
                        "_______٩(ఠ益ఠ)۶_______\n");
                rc = recv((int) socket, buf, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);
                pick = buf[0];
                break;
            }

            case '1': {
                memset(buf, 0, BUF_SIZE);

                char out[BUF_SIZE] = "\n_______٩(ఠ益ఠ)۶_______\n";
                strcat(out, topics);
                strcat(out, "_______٩(ఠ益ఠ)۶_______\n");
                to_client((int) socket, out);

                rc = recv((int) socket, buf, BUF_SIZE, 0);
                char pop[BUF_SIZE] = "";
                strcat(pop, buf);

                if (rc <= 0)
                    SentErrServer(REC);
                if (buf[0] == '0') {
                    pick = '0';
                    break;
                }
                if (list_caption(buf, (int) socket) == 1)
                    to_client((int) socket, "\n!ERROR! there is no such topic !ERROR!");

                rc = recv((int) socket, buf, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);
                if (buf[0] == '0') {
                    pick = '0';
                    break;
                }

                strcat(pop, "_");
                strcat(pop, buf);

                if (show_text(pop, (int) socket) == 1)
                    to_client((int) socket, "\n!ERROR! there is no such news !ERROR!");
//                else
//                    to_client((int) socket, "4 main menu '0'");


                rc = recv((int) socket, buf, BUF_SIZE, 0);
                if (buf[0] == '0') {
                    pick = '0';
                    break;
                }
                pick = '0';
                break;
            }

            case '2': {
                memset(buf, 0, BUF_SIZE);
                char topic[BUF_SIZE];
                char text[BUF_SIZE];

                to_client((int) socket, "\nenter topic:");
                rc = recv((int) socket, topic, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);

                to_client((int) socket, "\nenter caption:");
                rc = recv((int) socket, caption, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);


                to_client((int) socket, "\nenter text:");
                rc = recv((int) socket, text, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);


                add_news(topic, caption , text);
                pick = '0';
                break;

            }

            case '3': {
                printf("%d\n", (int) socket);

                to_client((int) socket, "#");
                DeleteClient(FindBySocket((int) socket));
                pthread_exit(NULL);
            }

            default: {
                SendErrorToClient((int) socket);
                pick = '0';
            }
        }
        memset(buf, 0, BUF_SIZE);
    }
}


void SendErrorToClient(int socket) {
    int rc = 0;
    rc = send((int) socket, "^", 1000, 0);
    if (rc <= 0)
        perror("send call failed");
}


int DeleteClient(int number) {

    if (number != -1) {

        to_client(users[number].s1, "#");

        if (number != threads) {
            users[number] = users[threads];
            memset(&users[threads], NULL, sizeof(users[threads]));
        }
        threads--;

        return 0;
    }
    return 1;
}


void list_topics() {
    *topics = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *fp;
    fp = fopen(TITLES, "r");
    if (fp == NULL)
        exit(1);

    int q = 0;
    while ((read = getline(&line, &len, fp)) != -1){
        q++;
        sprintf(chch, "%d", q);
        strncat(topics, chch, strlen(chch));
        strcat(topics, ". ");
        strcat(topics, line);
    }

    fclose(fp);
}


int show_text(char buf[], int socket) {
    char file[BUF_SIZE] = "";
    strcat(file, buf);
    strcat(file, ".txt");

    char out[BUF_SIZE] = "\n";
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *fp;

    if ((fp = fopen(file, "r")) == NULL)
        return 1;

    while ((read = getline(&line, &len, fp)) != -1)
        strcat(out, line);

    fclose(fp);
    to_client(socket, out);
    return 0;
}

int list_caption(char buf[], int socket) {
    char file[BUF_SIZE] = "";
    strcat(file, buf);
    strcat(file, ".txt");

    char out[BUF_SIZE] = "\n";
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *fp;

    if ((fp = fopen(file, "r")) == NULL)
        return 1;

    int q = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        q++;
        sprintf(chch, "%d", q);
        strncat(out, chch, strlen(chch));
        strcat(out, ". ");
        strcat(out, line);
    }
    fclose(fp);
    to_client(socket, out);
    return 0;
}


int amt_caption(char cap[]) {
    char file[BUF_SIZE] = "";
    strcat(file, cap);
    strcat(file, ".txt");

    char out[BUF_SIZE] = "\n";
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int num = 0;

    FILE *fp;

    if ((fp = fopen(file, "r")) == NULL)
        return 1;

    while ((read = getline(&line, &len, fp)) != -1)
        num++;

    fclose(fp);
    to_client(socket, out);
    return num;
}



int *FindBySocket(int socket) {
    int i = 0;
    for (i; i <= threads; i++)
        if (users[i].s1 == socket)
            return i;
}


void SentErrServer(char *s) {
    perror(s);
    exit(1);
}

void add_news(char *topic, char *caption, char *text) {

    FILE *fp1;
    fp1 = fopen(TITLES, "a");
    if (fp1 == NULL)
        exit(1);

    fprintf(fp1, "%s\n", topic);
    fclose(fp1);
    list_topics();



    char file_name[BUF_SIZE] = "";
    strcat(file_name, topic);
    strcat(file_name, ".txt");

    FILE *fp2;
    fp2 = fopen(file_name, "a");
    if (fp2 == NULL)
        exit(1);

    fprintf(fp2, "%s\n", caption);
    fclose(fp2);
    
    int nn = amt_caption(topic);

    char file_name_2[BUF_SIZE] = "";
    strcat(file_name_2, topic);
    strcat(file_name_2, "_");
    sprintf(chch, "%d", nn);
    strncat(file_name_2, chch, strlen(chch));

    strcat(file_name_2, ".txt");


    FILE *fp3;
    fp3 = fopen(file_name_2, "a");
    if (fp3 == NULL)
        exit(1);

    fprintf(fp3, "%s\n", text);
    fclose(fp3);
}

