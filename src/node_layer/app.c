#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "../../inc/util/mq_utils.h"

// filas
mqd_t m_queue_r;
mqd_t m_queue_w;

// struct mq_attr attr_r;
// struct mq_attr attr_w;

char file_name[100];

// nomes das filas
char* READ_CLIENT_MQ = "/W_CLIENT_MESSAGE_QUEUE";
char* WRITE_CLIENT_MQ = "/R_CLIENT_MESSAGE_QUEUE";

char* READ_SERVER_MQ = "/W_SERVER_MESSAGE_QUEUE";
char* WRITE_SERVER_MQ = "/R_SERVER_MESSAGE_QUEUE";

pid_t link_layer;

typedef enum {client, server} type;

//fecha tudo, incluindo o processo
void close_all(int s) {
    mq_close(m_queue_r);
    mq_close(m_queue_w);
    kill(link_layer, SIGINT);
    exit(0);
}

void handle_file(char* filename) {
    struct stat path_stat;
    int n = stat(filename, &path_stat);

    if (n == -1) {
        printf("!! Arquivo não encontrado\n");
        return;
    }

    if (!S_ISREG(path_stat.st_mode)) {
        printf("!! Caminho não aponta para um arquivo.\n");
        return;
    }

    int file_len;
    char *file_content = read_file(filename, &file_len);
    send_msg_to_queue(m_queue_w, file_content, file_len, 200);
    free(file_content);
}

void listdir(char* directory) {
    printf("\033[0;33m");
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (d) {
        while( (dir = readdir(d)) != NULL ) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    printf("\033[0m");
}

void process_input(char* input) {
    if (!strncmp(input, "ls ", 3)) {
        listdir(input + 3);
        return;
    }
    if (!strcmp(input, "exit")) {
        close_all(SIGINT);
        return;
    }
    if (!strncmp(input, "send ", 5)) {
        handle_file(input + 5);
        return;
    }
    printf("input inválido");
}

void* handle_user_input() {
    printf("Options:\n");
    printf(" - ls <directory_path>\n");
    printf(" - send <file_name>\n");
    printf(" - exit\n");

    char input[1024];
    while(1) {
        memset(input, 0x0, 1024);
        printf("app$ ");
        scanf(" %[^\n]", input);
        process_input(input);
    }

}

void* queue_receive_loop() {
    struct mq_attr attr = get_mqueue_attr(m_queue_r);
    char buffer[10000];
    long msg_len;
    while(1) {
        receive_msg_from_queue(m_queue_r, buffer, &msg_len, attr);
        if (msg_len > 0) {
            write_file(file_name, buffer, msg_len, "a+");
        }
    }
}

type process_command_line(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: ./app type link_process_id\n");
        printf("  type pode ser 'client' ou 'server'\n");
        printf("  link_process_id é o pid da camada de enlace\n");
        exit(0);
    }
    if (strcmp(argv[1], "client") && strcmp(argv[1], "server")) {
        printf("Segundo argumento inválido\n");
        exit(0);
    }

    link_layer = atoi(argv[2]);

    if (!strcmp(argv[1], "client")) {
        return client;
    }
    else {
        return server;
    }
}

void connect_to_queue(type t) {
    if (t == client) {
        m_queue_r = get_mq(READ_CLIENT_MQ, 0);
        m_queue_w = get_mq(WRITE_CLIENT_MQ, 0);
        strcpy(file_name, "/tmp/client.out");
    }
    else {
        m_queue_r = get_mq(READ_SERVER_MQ, 0);
        m_queue_w = get_mq(WRITE_SERVER_MQ, 0);
        strcpy(file_name, "/tmp/server.out");
    }
}

int main(int argc, char** argv) {

    signal(SIGINT, &close_all);
    type t = process_command_line(argc, argv);

    connect_to_queue(t);

    pthread_t mq_reading_stream;
    pthread_t user_input;

    pthread_create(&user_input, NULL, &handle_user_input, NULL);
//     pthread_create(&mq_reading_stream, NULL, &queue_receive_loop, NULL);

    queue_receive_loop();

    pthread_join(user_input, NULL);
    return 0;
}
