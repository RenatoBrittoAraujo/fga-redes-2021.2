#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_MSG_BLOCK_SIZE 10
#define MAX_MSG 100
#define FRAME_PARITY_BIT_SIZE 1
#define FRAME_INDEX_BIT_SIZE 64

void prefix()
{
    printf("[receiving_node] ");
}

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[MAX_MSG_BLOCK_SIZE * 2];
};

void printframe(char *str, char *frame, int frame_size)
{
    printf("%s:\t", str);
    int charv = sizeof(char) * 8;
    for (int i = 0; i < frame_size; i++)
    {
        if (frame[i / charv] & (1UL << (i % charv)))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

void packets_to_file(char **frames, int frame_count, int frame_content_size, char **file_content_o)
{
    int CHAR_BITS = sizeof(char) * 8;

    int filep = 0;
    int file_capacity = 4;
    char *file_content = (char *)malloc(sizeof(char) * file_capacity);

    for (uint64_t frame_index = 0; frame_index < frame_count; frame_index++)
    {
        /*
           Começa a manipulação dos bits, capaz de tornar pacotes em arquivo.

           NÂO ESTÁ OTIMIZADO, está feito assim para facilitar entendimento do avaliador ( e o meu :) )
        */

        if (file_capacity * CHAR_BITS < filep * CHAR_BITS + frame_content_size)
        {
            file_capacity *= 2;
            file_content = (char *)realloc(file_content, sizeof(char) * file_capacity);
        }

        // Recupera conteúdo do frame
        for (int fi = 0; fi < frame_content_size; fi++)
        {
            int framebit = (1UL << (fi % CHAR_BITS)) & frames[frame_index][fi / CHAR_BITS];
            if (framebit)
            {
                file_content[filep / CHAR_BITS] |= (1 << (filep % CHAR_BITS));
            }
            filep++;
        }
    }
    file_content[filep] = '\0';

    // Valores de retorno
    (*file_content_o) = file_content;
}

int is_end_frame(char *frame)
{
    int CHAR_BITS = sizeof(char) * 8;
    int frame_size = strlen(frame);
    for (int i = 0; i < frame_size; i++)
    {
        if (!(frame[i / CHAR_BITS] & (1 << (i % CHAR_BITS))))
        {
            return 0;
        }
    }
    return 1;
}

void send_message_buffer(char *data, int data_size)
{
    int frame_count = 1 + ((data_size - 1) / MAX_MSG_BLOCK_SIZE); // int ceil of data_size / MAX_MSG_BLOCK_SIZE

    key_t key;
    int msgid;
    key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT | IPC_PRIVATE);

    { // send header message (with frame count)
        struct mesg_buffer message;
        message.mesg_type = 4;
        char size_buffer[MAX_MSG_BLOCK_SIZE + 1];
        sprintf(size_buffer, "%d", frame_count + 1);
        strcpy(message.mesg_text, size_buffer);
        msgsnd(msgid, &message, sizeof(message), 0);
    }

    int index = 0;
    while (index < frame_count)
    {
        struct mesg_buffer message;
        char msg[100];
        message.mesg_type = 4;

        for (int i = MAX_MSG_BLOCK_SIZE * index;
             i < data_size && i < MAX_MSG_BLOCK_SIZE * (index + 1);
             i++)
        {
            int message_index = i - MAX_MSG_BLOCK_SIZE * index;
            msg[message_index] = data[i];
        }

        if (index == frame_count - 1)
        {
            msg[data_size - MAX_MSG_BLOCK_SIZE * index] = '\0';
        }
        else
        {
            msg[10] = '\0';
        }

        strcpy(message.mesg_text, msg);

        // send message
        msgsnd(msgid, &message, sizeof(message), 0);

        // await confirmation from receiver process
        msgrcv(msgid, &message, sizeof(message), 4, 0);
        index++;
    }

    // destroy queue
    msgctl(msgid, IPC_RMID, NULL);
}

int main(int argc, char *argv[])
{
    char *host = argv[2];
    char *port = argv[3];
    int frame_content_size = atoi(argv[1]);

    int sd, rc, n, tam_Cli;
    struct sockaddr_in endCli;  /* Vai conter identificacao do cliente */
    struct sockaddr_in endServ; /* Vai conter identificacao do servidor local */
    char msg[MAX_MSG];          /* Buffer que armazena os dados que chegaram via rede */

    if (argc < 3)
    {
        prefix();
        printf("Digite IP e Porta para este servidor\n");
        exit(1);
    }
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        prefix();
        printf("%s: nao pode abrir o socket \n", argv[0]);
        exit(1);
    }

    endServ.sin_family = AF_INET;
    endServ.sin_addr.s_addr = inet_addr(host);
    endServ.sin_port = htons(atoi(port));

    rc = bind(sd, (struct sockaddr *)&endServ, sizeof(endServ));
    if (rc < 0)
    {
        prefix();
        printf("%s: nao pode fazer bind na porta %s \n", argv[0], host);
        exit(1);
    }

    prefix();
    printf("%s: esperando por dados no IP: %s, porta UDP numero: %s\n", argv[0], argv[1], argv[2]);
    int frame_count = 0;
    int frame_capacity = 2;
    char **frames = (char **)malloc(sizeof(char *) * frame_capacity);
    int frame_size = frame_content_size + FRAME_INDEX_BIT_SIZE + FRAME_PARITY_BIT_SIZE;

    while (1)
    {
        memset(msg, 0x0, MAX_MSG);
        tam_Cli = sizeof(endCli);
        n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *)&endCli, &tam_Cli);
        if (n < 0)
        {
            prefix();
            printf("%s: nao pode receber dados \n", argv[0]);
            continue;
        }

        if (is_end_frame(msg))
        {
            break;
        }
        else
        {
            if (frame_count == frame_capacity)
            {
                frame_capacity *= 2;
                frames = (char **)realloc(frames, sizeof(char *) * frame_capacity);
            }
            frames[frame_count] = (char *)malloc(sizeof(char) * strlen(msg));
            strcpy(frames[frame_count], msg);
            frame_count++;
        }
    }

    prefix();
    printf("Pacotes recebidos (menos o pacote de encerramento) = %d \n", frame_count);

    char *file_content;
    packets_to_file(frames, frame_count, frame_content_size, &file_content);

    prefix();
    printf("Conteúdo do arquivo = %s\n", file_content);
    int file_size = strlen(file_content);

    send_message_buffer(file_content, file_size);

    return 0;
}
