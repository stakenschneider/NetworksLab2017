//Server trade

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

//#include <time.h>
//#include <string.h>


#define PORT 5001
#define BUF_SIZE 1000
#define TITLES "topics.txt"

void SendToClient(int socket, char* message); //для сообщений клиенту

void SeeTopics();
void NewNews(char *name, char *price);
int DeleteClient(char name[]); // удаление пользователя


void *ClientHandler(void* socket);
void *ServerHandler(void* empty);

int FindNumberByName(char name[]); //найти номер сокета по имени
//char *FindNameBySocket(int socket); //найти имя по сокету

void SentErrServer(char *s);

int SeeTopic(char filename[], int socket);
void WriteMessages(char filename[], int socket, char buf[]);
void EndTrade();


//void SendErrorToClient(int socket);
//bool manager_count = false; // if manager online
//char online_command[] = "online";


int threads = -1; //счетчик потоков
char topic_names[BUF_SIZE]; //список с темами
char kill_command[] = "kill";
char shutdown_command[] = "shutdown";

struct clients {
    char login[BUF_SIZE];
    int s1;
} *users;

int s;

int main(void) {

    users = (char*) malloc(sizeof (char));

    if (users == NULL) {
        EndTrade();
        exit(1);
    }

    printf("Server trade is working...\n");

    //Initialization
    struct sockaddr_in local, si_other;
    int s1, rc, slen = sizeof (si_other);

    //fill local
    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    //make socket
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        SentErrServer("Socket call failed");

    //attach port
    rc = bind(s, (struct sockaddr *) &local, sizeof (local));
    if (rc < 0)
        SentErrServer("Bind call failure");

    //thread options
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    //thread for server
    pthread_t server_thread;
    rc = pthread_create(&server_thread, &threadAttr, ServerHandler, (void*) NULL);

    //listening socket
    rc = listen(s, 5);
    if (rc)
        SentErrServer("Listen call failed");
    SeeTopics();

    //waiting for clients
    while (1) {
        //get connection
        s1 = accept(s, (struct sockaddr *) &si_other, &slen);

        if (s1 < 0)
            SentErrServer("Accept call failed");
        //Making new user struct

//        users[threads + 1].ip = inet_ntoa(si_other.sin_addr);
//        users[threads + 1].port = ntohs(si_other.sin_port);
        users[threads + 1].s1 = s1;
        printf("new socket=%d\n", (int) s1);
        //New thread
        pthread_t client_thread;

        rc = pthread_create(&client_thread, &threadAttr, ClientHandler, (void*) s1);

        if (rc != 0)
            SentErrServer("Creating thread false");
        threads++;
    }
    return 0;
}




void SendToClient(int socket, char* message) {
    int rc;
    rc = send(socket, message, BUF_SIZE, 0);

    if (rc <= 0)
        perror("send call failed");
}

void NewNews(char *name, char *topic) {

    char file_name[BUF_SIZE] = "";
    strcat(file_name, topic);
    strcat(file_name, ".txt");


    FILE *fp1;
    fp1 = fopen(file_name, "a");
    if (fp1 == NULL) {
        EndTrade();
        exit(1);
    }

    fprintf(fp1, "%s\n", name);
    fclose(fp1);
//    SeeTopics(); //refresh
}










void *ServerHandler(void* empty) {
    char text[40]; //buffer
    //Getting text from keyboard
    while (1) {
        gets(text);
        if (strstr(kill_command, text) != NULL) {
            char name[] = "";
            strcat(name, &text[5]);
            printf("%s\n", name);
            if (DeleteClient(name) == 0)
                printf("All right. %s was killed\n", name);
            else
                printf("Bad comand. Please try again\n");
        }

//        if (strstr(online_command, text) != NULL) {
//            WhoIsOnline(-1);
//        }

        if (strstr(shutdown_command, text) != NULL) {
            EndTrade();
        }
    }
}






void *ClientHandler(void* socket) {
    printf("New user login is:\n");
    int rc;
    char buf[ BUF_SIZE ]; //Buffer
//    char manager[ BUF_SIZE ] = "manager"; //Buffer
//    bool is_manager;

//    SendToClient((int) socket, "print u name:");
//
//
//    //recive login
//    rc = recv((int) socket, buf, BUF_SIZE, 0);
//    if (rc <= 0)
//        SentErrServer("Recv call failed");

//    //is name = manager?
//    int i = 0;
//    is_manager = true;
//    while (buf[i] != NULL) {
//        if (buf[i] != manager[i]) {
//            is_manager = false;
//            break;
//        }
//        i++;
//    }
//
//
//    //delete if manager already exist
//    if (is_manager == true) {
//        if (manager_count == true) {
//            printf("Client was Deleted. Manager already exist\n");
//            SendToClient((int) socket, "#Manager already exist");
//            threads--;
//            pthread_exit(NULL);
//        }
//    }
//
//
//
//
//    //saving login
//    int j = 0;
//    for (j; j <= threads; j++) {
//        if (users[j].s1 == (int) socket) {
//            if (is_manager == true) {
//                if (manager_count == false)//save manager
//                {
//                    users[j].manager == true;
//                    manager_count = true;
//                }
//            }
//            int i = 0;
//            while (buf[i] != NULL) {
//                users[j].login[i] = buf[i];
//                i++;
//            }
//            printf("%s\n", users[threads].login);
//        }
//    }
//    printf("\n");


    //Working
    char pick = '0';
    while (1) {
        switch (pick) {
            case '0':
            {
                SendToClient((int) socket, "|=====Commands:==============|=====Semantic:===========================|\n"
                        "| see news                   | 1                                       |\n"
                        "| add news                   | 2                                       |\n"
                        "| disconnect user            | 3                                       |\n"
                        "|============================|=========================================|\n");


                rc = recv((int) socket, buf, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer("Recv call failed when pick was 0");
                pick = buf[0];
                break;
            }

            case '1'://See news
            {
                memset(buf, 0, BUF_SIZE);
                char out[BUF_SIZE] = "If you want to open topic, type name of the lot\n";
                strcat(out, topic_names);
                SendToClient((int) socket, out); //send topic
                rc = recv((int) socket, buf, BUF_SIZE, 0); //Reading new point of menu


                if (rc <= 0)
                    SentErrServer("Recv call failed");

                //If client want to back
                if (buf[0] == '0') {
                    pick = '0';
                    break;
                }

                char filename[] = "";
                strcat(filename, buf);
                strcat(filename, ".txt");

                if (SeeTopic(filename, (int) socket) == 1) {
                    pick = '0';
                    break;
                }



                char buf2[ BUF_SIZE ]; //New buffer only for writing message
                rc = recv((int) socket, buf2, BUF_SIZE, 0); //Reading new message to write
                if (rc <= 0)
                    SentErrServer("Recv call failed");

                if ((buf2[0] == '0') && (strlen(buf2) == 1)) { //If client want to back
                    pick = '0';
                    break;
                } else {
                    WriteMessages(filename, (int) socket, buf2);
                    pick = '0';
                    break;
                }
            }


            case '2'://New news
            {
                memset(buf, 0, BUF_SIZE);
                char name[ BUF_SIZE ];
                char topic[ BUF_SIZE ];

                SendToClient((int) socket, "Please write news\n");
                rc = recv((int) socket, name, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer("Recv call failed");

                SendToClient((int) socket, "Please write topic of the lot\n");
                rc = recv((int) socket, topic, BUF_SIZE, 0);
                if (rc <= 0)
                    SentErrServer("Recv call failed");

                NewNews(name, topic);
                pick = '0';
                break;
            }

            case '3'://Disconnect client
            {
                printf("%d\n", (int) socket);
                SendToClient((int) socket, "#");
//                int a = DeleteClient(FindNameBySocket((int) socket));
                pthread_exit(NULL);
//                pick = '0';
//                break;
            }

            default:
            {
            }
        }
        memset(buf, 0, BUF_SIZE);
    }



}

//void SendErrorToClient(int socket) {
//    int rc = 0;
//    rc = send((int) socket, "^", 1000, 0);
//    if (rc <= 0)
//        perror("send call failed");
//}

int DeleteClient(char name[]) {
    printf("DeleteClient running...\n");
    int number;
    number = FindNumberByName(name);
    //printf("%d!\n",number);
    if (number != -1) {
//        if (users[number].manager == true)
//            manager_count = false;
        //Send signal for client
        SendToClient(users[number].s1, "#");
        //moving cells
        //printf("%d,%d\n",number, threads);
        if (number != threads) //if it's not the last thread
        {
            users[number] = users[threads];
            memset(&users[threads], NULL, sizeof (users[threads]));
        }
        threads--;

        return 0;
    }
    return 1;
}

int FindNumberByName(char name[]) {
    int j = 0;
    int i = 0;
    int rightflag;
    for (j; j <= threads; j++) {
        rightflag = 1;
        i = 0;
        while (name[i] != NULL) {
            if (users[j].login[i] != name[i]) {
                rightflag = 0;
            }
            i++;
        }
        if (rightflag == 1) {
            return j;
        }
    }
    return -1;
}



void SeeTopics() {
    *topic_names = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *fp;
    fp = fopen(TITLES, "r");
    if (fp == NULL) {
        EndTrade();
        exit(1);
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        strcat(topic_names, line);
    }
    fclose(fp);
}

int SeeTopic (char filename[], int socket) {
    size_t len = 0;
    ssize_t read;
    char *line = NULL;
    char out[BUF_SIZE] = "";
    int rc;
    FILE *fp;
    //printf("%s filename\n",filename);
    if ((fp = fopen(filename, "r")) == NULL)
        return 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        strcat(out, line);
    }
    fclose(fp);
    strcat(out, "You can write new price or 0 to back");
    SendToClient(socket, out);
    return 0;
}

void WriteMessages(char filename[], int socket, char buf[]) {

        FILE *fp1;
        fp1 = fopen(filename, "r");
        if (fp1 == NULL) {
            EndTrade();
            exit(1);
        }
        size_t len = 0;
        ssize_t read;
        char *line = NULL;
        while ((read = getline(&line, &len, fp1)) != -1) {
        }

        char last_price[100] = {};

        int i = 0;
        while (line[i] != '<') {
            i++;
        }

        memcpy(last_price, line + 2, (i - 2) * sizeof (char));

        printf("%s\n", last_price);

        int last_price_int = atoi(last_price);
        int new_price_int = atoi(buf);

        if (new_price_int <= last_price_int) {
            fclose(fp1);
            SendToClient(socket, "New price <= old price!\n");
        } else {
            fclose(fp1);
            printf("last_price=%s\n", last_price);

            //For local time
            char li[50];
            time_t rawtime;
            struct tm * timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(li, 15, "%d/%m/%Y", timeinfo);

            FILE *fp;
            fp = fopen(filename, "a");
            if (fp == NULL) {
                EndTrade();
                exit(1);
            }
            //printf("%s",filename);
//            fprintf(fp, "--%s<%s> %s\n", buf, FindNameBySocket(socket), li);
            fclose(fp);
            SendToClient(socket, "All right!\n");
        }

}

//char *FindNameBySocket(int socket) {
//    int i = 0;
//    for (i; i <= threads; i++) {
//        if (users[i].s1 == socket)
//            return users[i].login;
//    }
//    return "error";
//}

void SentErrServer(char *s) //error handling
{
    perror(s);
    EndTrade();
    exit(1);
}

void EndTrade() {
    int i = 0;
    for (i; i <= threads; i++) {
        shutdown(users[i].s1, 2);
        close(users[i].s1);
    }

}