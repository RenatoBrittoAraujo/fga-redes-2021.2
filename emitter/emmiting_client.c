#include <stdio.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define MAX 10

char program_prefix[] = "[emmiting_client]";

void prefix()
{
    printf("%s ", program_prefix);
}

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[100];
} message;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        prefix();
        printf("Uso: %s <>", argv[0]);
    }

    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok("progfile", 65);

    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);
    message.mesg_type = 1;

    printf("Write Data : ");
    fgets(message.mesg_text, MAX, stdin);

    // msgsnd to send message
    msgsnd(msgid, &message, sizeof(message), 0);

    // display the message
    printf("Data send is : %s \n", message.mesg_text);

    return 0;
}