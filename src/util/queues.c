#include <stdio.h>
#include <sys/msg.h>

#include "queues.h"

char *get_client_message(const int queue_index)
{
    key_t key;
    int msgid;
    key = ftok("receiving", 69);
    msgid = msgget(key, 0666 | IPC_CREAT);

    int msgs_count = -1;
    int received_count = 0;
    struct mesg_buffer confirm_message;
    confirm_message.mesg_type = queue_index;

    char *message_str;

    while (received_count != msgs_count)
    {
        struct mesg_buffer message;

        // receive message
        msgrcv(msgid, &message, sizeof(message), queue_index, 0);

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

void send_message_buffer(const char *data, const int data_size, const int queue_index)
{
    int frame_count = 1 + ((data_size - 1) / MAX_MSG_BLOCK_SIZE); // int ceil of data_size / MAX_MSG_BLOCK_SIZE

    key_t key;
    int msgid;
    key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);

    { // send header message (with frame count)
        struct mesg_buffer message;
        message.mesg_type = queue_index;
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
        message.mesg_type = queue_index;

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
        msgrcv(msgid, &message, sizeof(message), queue_index, 0);
        index++;
    }

    // destroy queue
    msgctl(msgid, IPC_RMID, NULL);
}
