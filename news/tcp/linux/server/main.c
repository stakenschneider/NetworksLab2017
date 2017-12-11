#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5001
#define BUF_SIZE 1000
#define TITLES "topics.txt"
#define REC "Receive call failed"

void *ClientHandler(void *socket);
void *ServerHandler();

void SendErrorToClient(int socket);
void SentErrServer(char *s);

void AddNews(char *topic, char *caption, char *text);
int DeleteClient(int number);

int *FindBySocket(int socket);

void ReadTopics();
int ReadText(char buf[], int socket);
int ReadCaption(char buf[], int socket);

int ReadNumCap(char cap[]);

void SendToClient(int socket, char *message);

void EndTrade();

int s;
int threads = -1;
char topics[BUF_SIZE];
char caption[BUF_SIZE];
char kill_command[] = "kill";
char shutdown_command[] = "shutdown";

struct clients {
    int s1;
} *users;


int main(void) {

    users = (char *) malloc(sizeof(char));
    if (users == NULL) {
        EndTrade();
        exit(1);
    }

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
    ReadTopics();

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

        if (strstr(shutdown_command, text) != NULL) {
            EndTrade();
        }
    }
}


void SendToClient(int socket, char *message) {
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
                SendToClient((int) socket, "\n_______٩(ఠ益ఠ)۶_______\n"
                        "1. посмотреть список тем\n"
                        "2. добавить новость\n"
                        "3. выход\n"
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
                SendToClient((int) socket, out);

                rc = recv((int) socket, buf, BUF_SIZE, 0);
                char pop[BUF_SIZE] = "";
                strcat(pop, buf);

                if (rc <= 0)
                    SentErrServer(REC);
                if (buf[0] == '0') {
                    pick = '0';
                    break;
                }
                if (ReadCaption(buf, (int) socket) == 1)
                    SendToClient((int) socket, "\n!ERROR! такой темы нет !ERROR!");

                rc = recv((int) socket, buf, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);
                if (buf[0] == '0') {
                    pick = '0';
                    break;
                }

                strcat(pop, "_");
                strcat(pop, buf);

                if (ReadText(pop, (int) socket) == 1)
                    SendToClient((int) socket, "\n!ERROR! такой новости нет !ERROR!");
                else
                    SendToClient((int) socket, "для выхода в главное меню нажмите '0'");


                rc = recv((int) socket, buf, BUF_SIZE, 0);
                if (buf[0] == '0') {
                    pick = '0';
                    break;
                }
                pick = '0';
                break;
//                } else{
//                    if (buf[0] == '0') {
//                        pick = '0';
//                        break;
//                    }
//                }
//                break;
            }

            case '2': {
                memset(buf, 0, BUF_SIZE);
                char topic[BUF_SIZE];
                char text[BUF_SIZE];

                SendToClient((int) socket, "\nназвание темы:");
                rc = recv((int) socket, topic, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);

                SendToClient((int) socket, "\nзаголовок новости:");
                rc = recv((int) socket, caption, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);


                SendToClient((int) socket, "\nтекст новости:");
                rc = recv((int) socket, text, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer(REC);


                AddNews(topic, caption , text);
                pick = '0';
                break;

            }

            case '3': {
                printf("%d\n", (int) socket);

                SendToClient((int) socket, "#");
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

        SendToClient(users[number].s1, "#");

        if (number != threads) {
            users[number] = users[threads];
            memset(&users[threads], NULL, sizeof(users[threads]));
        }
        threads--;

        return 0;
    }
    return 1;
}


void ReadTopics() {
    *topics = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *fp;
    fp = fopen(TITLES, "r");
    if (fp == NULL) {
        EndTrade();
        exit(1);
    }

    int q = 0;
    while ((read = getline(&line, &len, fp)) != -1){
        q++;
        //TODO: int to char?
//        strcat(topics, q + ". ");
        strcat(topics, line);
    }

    fclose(fp);
}


int ReadText(char buf[], int socket) {
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

    while ((read = getline(&line, &len, fp)) != -1) {
        strcat(out, line);
    }
    fclose(fp);
    SendToClient(socket, out);
    return 0;
}

int ReadCaption(char buf[], int socket) {
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
        //TODO: int to char?
//        strcat(out, q + ". ");
        strcat(out, line);
    }
    fclose(fp);
    SendToClient(socket, out);
    return 0;
}


int ReadNumCap(char cap[]) {
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

    while ((read = getline(&line, &len, fp)) != -1) {
        num++;
    }

    fclose(fp);
    SendToClient(socket, out);
    return num;
}



int *FindBySocket(int socket) {
    int i = 0;
    for (i; i <= threads; i++) {
        if (users[i].s1 == socket)
            return i;
    }
}


void SentErrServer(char *s) {
    perror(s);
    EndTrade();
    exit(1);
}

void AddNews(char *topic, char *caption, char *text) {

    char file_name[BUF_SIZE] = "";
    strcat(file_name, topic);
    strcat(file_name, ".txt");

    FILE *fp1;
    fp1 = fopen(TITLES, "a");
    if (fp1 == NULL) {
        EndTrade();
        exit(1);
    }
    fprintf(fp1, "%s\n", topic);
    fclose(fp1);
    ReadTopics();

    FILE *fp2;
    fp2 = fopen(file_name, "a");
    if (fp2 == NULL) {
        EndTrade();
        exit(1);
    }
    fprintf(fp2, "%s\n", text);
    fclose(fp2);

    int nn = ReadNumCap(topic);
    char file_name_2[BUF_SIZE] = "";
    strcat(file_name_2, topic);
    strcat(file_name_2, "_");

    //TODO: int to char?
    strcat(file_name_2, nn);
    strcat(file_name_2, ".txt");


    FILE *fp3;
    fp3 = fopen(file_name, "a");
    if (fp3 == NULL) {
        EndTrade();
        exit(1);
    }
    fprintf(fp3, "%s\n", caption);
    fclose(fp3);
}


void EndTrade() {
    int i = 0;
    for (i; i <= threads; i++) {
        shutdown(users[i].s1, 2);
        close(users[i].s1);
    }
}