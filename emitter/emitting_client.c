#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_MSG_BLOCK_SIZE 10

void prefix()
{
    printf("[emmiting_client] ");
}

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[MAX_MSG_BLOCK_SIZE * 2];
};

char *read_file(char *filename)
{
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");

    if (handler)
    {
        fseek(handler, 0, SEEK_END);
        string_size = ftell(handler);
        rewind(handler);
        buffer = (char *)malloc(sizeof(char) * (string_size + 1));
        read_size = fread(buffer, sizeof(char), string_size, handler);
        buffer[string_size] = '\0';
        if (string_size != read_size)
        {
            free(buffer);
            buffer = NULL;
        }
        fclose(handler);
    }
    return buffer;
}

void send_message_buffer(char *data, int data_size)
{
    // int ceil of data_size / MAX_MSG_BLOCK_SIZE
    int frame_count = 1 + ((data_size - 1) / MAX_MSG_BLOCK_SIZE);

    key_t key;
    int msgid;
    key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);

    { // send header message (with frame count)
        struct mesg_buffer message;
        message.mesg_type = 1;
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
        message.mesg_type = 1;

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
        msgrcv(msgid, &message, sizeof(message), 2, 0);
        index++;
    }

    // destroy queue
    msgctl(msgid, IPC_RMID, NULL);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        prefix();
        printf("Uso: %s <filepath>\n", argv[0]);
        exit(1);
    }

    char *filedata = read_file(argv[1]);
    if (!filedata)
    {
        prefix();
        printf("Erro ao ler arquivo %s, abortando\n", argv[1]);
        exit(2);
    }
    else
    {
        prefix();
        printf("Arquivo %s lido: %s\n", argv[1], filedata);
    }
    int filedata_size = strlen(filedata);

    send_message_buffer(filedata, filedata_size);

    free(filedata);

    return 0;
}