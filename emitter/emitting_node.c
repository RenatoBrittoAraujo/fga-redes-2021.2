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

#define MAX_MSG_BLOCK_SIZE 10

void prefix()
{
    printf("[emmiting_node] ");
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
    key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);

    int msgs_count = -1;
    int received_count = 0;
    struct mesg_buffer confirm_message;
    confirm_message.mesg_type = 2;

    char *message_str;

    while (received_count != msgs_count)
    {
        struct mesg_buffer message;

        // receive message
        msgrcv(msgid, &message, sizeof(message), 1, 0);

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

void transmit_file(char *filecontent, int argc, char *argv[])
{
    char *ip = argv[1];
    char *host = argv[2];

    int sd, rc, i;
    struct sockaddr_in ladoCli;
    struct sockaddr_in ladoServ;

    ladoServ.sin_family = AF_INET;
    ladoServ.sin_addr.s_addr = inet_addr(ip);
    ladoServ.sin_port = htons(atoi(host));

    ladoCli.sin_family = AF_INET;
    ladoCli.sin_addr.s_addr = htonl(INADDR_ANY);
    ladoCli.sin_port = htons(0);

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        prefix();
        printf("%s: n�o pode abrir o socket \n", argv[0]);
        exit(1);
    }

    rc = bind(sd, (struct sockaddr *)&ladoCli, sizeof(ladoCli));
    if (rc < 0)
    {
        prefix();
        printf("%s: n�o pode fazer um bind da porta\n", argv[0]);
        exit(1);
    }
    prefix();
    printf("Conexão emissora {UDP, IP_Cli: %s, Porta_Cli: %u, IP_R: %s, Porta_R: %s}\n", inet_ntoa(ladoCli.sin_addr), ntohs(ladoCli.sin_port), argv[1], argv[2]);

        for (i = 3; i < argc; i++)
    {
        rc = sendto(sd, argv[i], strlen(argv[i]), 0, (struct sockaddr *)&ladoServ, sizeof(ladoServ));
        if (rc < 0)
        {
            prefix();
            printf("%s: nao pode enviar dados %d \n", argv[0], i - 1);
            close(sd);
            exit(1);
        }
        prefix();
        printf("Enviando parametro %d: %s\n", i - 2, argv[i]);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        prefix();
        printf("Uso correto: %s <ip_do_servidor> <porta_do_servidor> <dado1> ... <dadoN> \n", argv[0]);
        exit(1);
    }

    char *message_str = get_client_message();

    prefix();
    printf("Arquivo recebido do cliente: %s\n", message_str);

    transmit_file(message_str, argc, argv);

    free(message_str);

    return 0;
}
