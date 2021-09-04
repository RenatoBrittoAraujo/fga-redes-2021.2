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
#define FRAME_PARITY_BIT_SIZE 1
#define FRAME_INDEX_BIT_SIZE 64

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
    key = ftok("receiving", 69);
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

void printcharbit(const char *str, char c)
{
    printf("%s: ", str);
    for (int i = 0; i < sizeof(char) * 8; i++)
    {
        if (c & (1UL << i))
            printf("1");
        else
            printf("0");
    }
    printf("\n");
}

void printframe(char *str, char *frame, int frame_size)
{
    printf("%s:\t", str);
    int charv = sizeof(char) * 8;
    printf("|");
    for (int i = 0; i < frame_size; i++)
    {
        if (i == frame_size - 1 || i == frame_size - 65)
            printf("|");

        if (frame[i / charv] & (1UL << (i % charv)))
            printf("1");
        else
            printf("0");
    }
    printf("|");
    printf("\n");
}

void file_to_packets(char *file_content, int frame_content_size, char ***frames_o, int *frame_count_o)
{
    /*
        Aqui fazemos o processo de dividir a string em seus respectivos quadros e precalcular
        constantes
    */

    int CHAR_BITS = sizeof(char) * 8;

    int file_size = strlen(file_content);  // bytes
    int bit_count = file_size * CHAR_BITS; // bits

    int frame_size = frame_content_size + FRAME_INDEX_BIT_SIZE + FRAME_PARITY_BIT_SIZE; // bits

    int frame_count = 1 + ((bit_count - 1) / frame_content_size); // ceil bit_count / frame_content_size
    int frame_string_size = 1 + ((frame_size - 1) / CHAR_BITS);   // ceil frame_size / bits num char

    int filep = 0;

    char **frames = (char **)malloc(sizeof(char *) * (frame_count + 1));

    for (uint64_t frame_index = 0; frame_index < frame_count; frame_index++)
    {
        /*
           Começa a manipulação dos bits, capaz de gerar quadros de qualquer tamanho e enviando-os
           frame por frame.

           NÂO ESTÁ OTIMIZADO, está feito assim para facilitar entendimento do avaliador ( e o meu :) )
        */

        char *frame = (char *)malloc(sizeof(char) * frame_string_size);
        { // Seta todos os bits do frame para 0
            char zerochar = '?';
            for (int i = 0; i < CHAR_BITS; i++)
            {
                zerochar &= ~(1UL << i);
            }
            for (int i = 0; i < frame_string_size; i++)
            {
                frame[i] = zerochar;
            }
        }

        // Insere conteúdo do frame
        for (int fi = 0; fi < frame_content_size; fi++)
        {
            if (filep >= file_size * 8)
            {
                break;
            }
            int filebit = (1UL << (filep % CHAR_BITS)) & file_content[filep / CHAR_BITS];
            if (filebit)
            {
                frame[fi / CHAR_BITS] |= (1UL << (fi % CHAR_BITS));
            }
            filep++;
        }

        // Insere index do quadro
        for (int fi = frame_content_size; fi < frame_content_size + FRAME_INDEX_BIT_SIZE; fi++)
        {
            int intbit = !!((1LL << (fi - frame_content_size)) & frame_index);
            if (intbit)
            {
                frame[fi / CHAR_BITS] |= (1UL) << (fi % CHAR_BITS);
            }
        }

        int parity_bit = 0; // faz da soma de paridade final ser sempre par
        {
            for (int i = 0; i < frame_content_size + FRAME_INDEX_BIT_SIZE; i++)
            {
                int bitis1 = frame[i / CHAR_BITS] & (1UL << (i % CHAR_BITS));
                if (bitis1)
                {
                    parity_bit = !parity_bit;
                }
            }
        }

        // Insere bit de paridade
        int pos = frame_content_size + FRAME_INDEX_BIT_SIZE;
        if (parity_bit)
        {
            frame[pos / CHAR_BITS] |= (1 << (pos % CHAR_BITS));
        }

        // frame completo :) nossa essa função teve erro demais eu to exausto de debugar kk
        frames[frame_index] = frame;
    }

    // Add end frame
    frames[frame_count] = (char *)malloc(sizeof(char *) * frame_string_size);
    for (int i = 0; i < frame_size; i++)
    {
        frames[frame_count][i / CHAR_BITS] |= 1 << (i % CHAR_BITS);
    }

    // Valores de retorno
    (*frame_count_o) = frame_count + 1;
    (*frames_o) = frames;
}

void transmit_file(char *file_content, int argc, char *argv[])
{
    char *ip = argv[2];
    char *host = argv[3];
    int frame_content_size = atoi(argv[1]);

    /*
        ========= Código do professor
    */
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
        printf("%s: não pode abrir o socket \n", argv[0]);
        exit(1);
    }

    rc = bind(sd, (struct sockaddr *)&ladoCli, sizeof(ladoCli));
    if (rc < 0)
    {
        prefix();
        printf("%s: não pode fazer um bind da porta\n", argv[0]);
        exit(1);
    }
    /*
        AQUI NOS CONVERTERMOS O CONTEUDO DO ARQUIVO PARA PACOTES!
    */
    char **frames;
    int frame_count;
    file_to_packets(file_content, frame_content_size, &frames, &frame_count);
    prefix();
    printf("Pacotes enviados = %d\n", frame_count);

    for (int i = 0; i < frame_count; i++)
    {
        char v[10];
        sprintf(v, "Pacote %d", i);
        prefix();
        printframe(v, frames[i], frame_content_size + 64 + 1);
        rc = sendto(sd, frames[i], strlen(frames[i]), 0, (struct sockaddr *)&ladoServ, sizeof(ladoServ));
        if (rc < 0)
        {
            prefix();
            printf("%s: nao pode enviar dados %d \n", argv[0], i - 1);
            close(sd);
            exit(1);
        }
    }
    /*
        ========= Fim do código do professor
    */
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

    transmit_file(message_str, argc, argv);

    free(message_str);

    return 0;
}
