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
#define REC "Receive call failed"
#define MAIN_MENU "\n_______٩(ఠ益ఠ)۶_______\n 1. view list of topics\n 2. add news \n 3. exit\n_______٩(ఠ益ఠ)۶_______\n"
#define COMMANDS 7

void *ClientHandler(void *socket);
void *ServerHandler();

void SendErrorToClient(int socket);
void SentErrServer(char *s);

void add_news(char *topic, char *news, char *text);
void list_topics(int socket);
void list_caption(char buf[], int socket);
int show_text(char buf[], int socket);
void to_client(int socket, char *message);
void init();

int s, threads = -1;
int themes_count = -1;
char shutdown_command[] = "shutdown";

struct clients {
    int s1;
} *users;

static char *commands[] = {
        "start",
        "1",
        "theme",
        "news",
        "2",
        "3",
        "add",


};

struct news{
    char* name;
};

struct topic {
    char name[100];
    int count;
    struct news *names;
} *topics;

int main(void) {

    users = (char *) malloc(sizeof (char));
    if (users == NULL)
        exit(1);

    topics = (char *) malloc(sizeof (char));
    if (users == NULL)
        exit(1);

    printf("server is working\n");

    struct sockaddr_in local, si_other;
    int s1, rc, slen = sizeof (si_other);

    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        SentErrServer("socket call failed");

    rc = bind(s, (struct sockaddr *) &local, sizeof (local));
    if (rc < 0)
        SentErrServer("bind call failure");

    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    pthread_t server_thread;
    rc = pthread_create(&server_thread, &threadAttr, ServerHandler, (void *) NULL);
    init();

    rc = listen(s, 5);
    if (rc)
        SentErrServer("listen call failed");

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
    while (1) {
        char text[40];
        fgets(text, BUF_SIZE, stdin);
        if (!strcmp(shutdown_command, text)) {
            exit(1);
        }
    }
}

void to_client(int socket, char *message) {
    int rc;
    rc = send(socket, message, BUF_SIZE, 0);
    if (rc <= 0)
        perror("send call failed");
}

void *ClientHandler(void *arg) {
    int socket = (int) arg;
    char buf[BUF_SIZE];
    int rc;
    to_client(socket, "Для начала работы напишите start \n");
    while (1) {
        char pick[5] = "";
        int command = 0;
        rc = recv(socket, buf, BUF_SIZE, 0);
        if (rc <= 0)
            SentErrServer(REC);
        int position = 0;
        while ((buf[position] != ' ') && (buf[position] != '\n') && (buf[position] != NULL) && (position != 5)) {
            pick[position] = buf[position];
            position++;
        }
        while (command < COMMANDS) {

            if (!strcmp(commands[command], pick))
                break;
            command++;
        }
        position++;


        switch (command) {
            case 0:
            {
                to_client(socket, MAIN_MENU);
                break;
            }

            case 1: // Список тем
            {
                list_topics(socket);
                to_client(socket, "Чтобы посмотреть новости по теме введите: 'theme theme_name' \n");
                break;
            }

            case 2: // Cписок новостей в теме
            {

                char theme[BUF_SIZE] = "";
                int i = position;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    theme[i - position] = buf[i];
                    i++;
                }
                list_caption(theme, socket);
                to_client(socket, "Чтобы прочитать новость введите: 'news news_name' \n");
                break;
            }

            case 3: // Текст новости
            {
                char news[BUF_SIZE] = "";
                int i = position;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    news[i - position] = buf[i];
                    i++;
                }
                if (show_text(buf, (int) socket) == 1)
                    to_client((int) socket, "\n!ERROR! такой новости нет !ERROR! \n");
                break;
            }

            case 4: // Добавить новость
            {
                to_client(socket, "Чтобы добавить новость введите 'topic_name news_name text' \n");
                break;

            }

            case 5: // Отключиться
            {
                to_client((int) socket, "#");
                pthread_exit(NULL);
            }

            case 6:
            {
                memset(buf, 0, BUF_SIZE);
                char topic[BUF_SIZE];
                char news[BUF_SIZE];
                char text[BUF_SIZE];
                int i = position;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    topic[i - position] = buf[i];
                    i++;
                }
                i++;
                int j = 0;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    news[j] = buf[i];
                    i++;
                    j++;
                }
                i++;
                j = 0;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    text[j] = buf[i];
                    i++;
                    j++;
                }
                add_news(topic, news, text);
                break;
            }

            default:
            {
                SendErrorToClient((int) socket);
                break;
            }
        }
        memset(buf, 0, BUF_SIZE);
    }
}

void init() {

    strcpy(topics[0].name , "Политика");
    topics[0].names =  (char *) malloc(BUF_SIZE*sizeof(char));
    topics[0].count = 0;
    strcpy(topics[1].name , "Спорт");
    topics[1].names =  (char *) malloc(BUF_SIZE*sizeof(char));
    topics[1].count = 0;
    strcpy(topics[2].name , "Погода");
    topics[2].names =  (char *) malloc(BUF_SIZE*sizeof(char));
    topics[2].count = 0;
    strcpy(topics[3].name , "Финансы");
    topics[3].names =  (char *) malloc(BUF_SIZE*sizeof(char));
    topics[3].count = 0;
    strcpy(topics[4].name , "Технологии");
    topics[4].names =  (char *) malloc(BUF_SIZE*sizeof(char));
    topics[4].count = 0;
    themes_count = 4;

}

void SendErrorToClient(int socket) {
    int rc = 0;
    rc = send((int) socket, "^", 1000, 0);
    if (rc <= 0)
        perror("send call failed");
}

void list_topics(int socket) {
    char out[BUF_SIZE] = "Список тем \n";

    for (int i = 0; i <= themes_count; i++) {

        strcat(out, topics[i].name);
        strcat(out, "\n");

    }

    to_client(socket, out);
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

void list_caption(char *buf, int socket) {
    char out[BUF_SIZE] = "Новости в теме ";
    strcat(out, buf);
    strcat(out, ":\n");
    for (int i = 0; i <= themes_count; i++) {
        if (!strcmp(topics[i].name, buf)) {
            for (int j = 0; j < topics[i].count; j++) {
                strcat(out, topics[i].names[j].name);
                strcat(out, "\n");
            }
            if(!topics[i].count){
                strcat(out, "Тем нет");
            }
            to_client(socket, out);
            return;
        }
    }
    to_client(socket, "Нет такой темы");
}

void SentErrServer(char *s) {
    perror(s);
    exit(1);
}

void add_news(char *topic, char *news, char *text) {

    for (int i = 0; i <= themes_count; i++) {
        if (!strcmp(topics[i].name, topic)) {
            topics[i].count++;
            topics[i].names[i].name[topics[i].count] = news;
        }
    }
    char file_name[BUF_SIZE] = "";
    strcat(file_name, news);
    strcat(file_name, ".txt");
    FILE *fp3;
    fp3 = fopen(file_name, "a");
    if (fp3 == NULL)
        exit(1);
    fprintf(fp3, "%s\n", text);
    fclose(fp3);
}

