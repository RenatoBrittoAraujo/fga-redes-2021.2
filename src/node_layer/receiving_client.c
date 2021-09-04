#include <stdio.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdint.h>

#define MAX_MSG_BLOCK_SIZE 10

void prefix()
{
    printf("[receiving_client] ");
}

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[MAX_MSG_BLOCK_SIZE * 2];
};

char *get_client_message()
{
    key_t key;
    int msgid;
    key = ftok("receiving", 69);
    msgid = msgget(key, 0666 | IPC_CREAT);

    int msgs_count = -1;
    int received_count = 0;
    struct mesg_buffer confirm_message;
    confirm_message.mesg_type = 4;

    char *message_str;

    while (received_count != msgs_count)
    {
        struct mesg_buffer message;

        // receive message
        msgrcv(msgid, &message, sizeof(message), 4, 0);

        if (msgs_count == -1)
        {
            msgs_count = atoi(message.mesg_text);
            message_str = malloc(sizeof(char) * (MAX_MSG_BLOCK_SIZE * msgs_count + 1));
        }

        int message_size = strlen(message.mesg_text);
        for (int i = 0; i < message_size && i < MAX_MSG_BLOCK_SIZE; i++)
        {
            message_str[(received_count - 1) * MAX_MSG_BLOCK_SIZE + i] = message.mesg_text[i];
        }

        received_count++;

        if (received_count != msgs_count)
        {
            // release communication lock
            msgsnd(msgid, &confirm_message, sizeof(message), 0);
        }
        else
        {
            // destroy queue
            msgctl(msgid, IPC_RMID, NULL);
        }
    }

    return message_str;
}

int main(int argc, char **argv)
{
    char *file = get_client_message();

    prefix();
    printf("Arquivo = %s\n", file);
}