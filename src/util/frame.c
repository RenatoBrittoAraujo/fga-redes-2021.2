#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../inc/util/frame.h"

#include <errno.h>

static void print_msg(const char *prefix, char *buffer, int n)
{
    printf("%s", prefix);
    for (int i = 0; i < n; i++)
    {
        if (buffer[i] < 32 || buffer[i] > 126)
        {
            printf(" ");
        }
        else
        {
            printf("%c", buffer[i]);
        }
    }
    printf("\n");
}

char *frame_to_str(Frame frame, uint *buffer_len)
{
    char *buffer = (char *)malloc(HEADER_LEN + frame.packet.data_len + TRAILER_LEN);
    buffer[KIND_POS] = frame.kind;
    buffer[SEQ_POS] = frame.seq;
    buffer[ACK_POS] = frame.ack;
    memcpy(&buffer[DATA_LEN_POS], &frame.packet.data_len, sizeof(uint));
    //     fwrite(&buffer[DATA_LEN_POS], sizeof(char), 4, stdout);
    if (frame.packet.data_len > 0)
    {
        memcpy(&buffer[DATA_LEN_POS + sizeof(uint)], frame.packet.data, frame.packet.data_len);
    }
    memcpy(&buffer[DATA_LEN_POS + sizeof(uint) + frame.packet.data_len], &frame.trailer, sizeof(uint));

    *buffer_len = HEADER_LEN + frame.packet.data_len + TRAILER_LEN;
    return buffer;
}

Frame str_to_frame(char *buffer)
{
    Frame frame;
    frame.kind = buffer[KIND_POS];
    frame.seq = buffer[SEQ_POS];
    frame.ack = buffer[ACK_POS];
    memcpy(&frame.packet.data_len, &buffer[DATA_LEN_POS], sizeof(uint));

    if (frame.packet.data_len > 0)
    {
        //         print_msg("buffer: ", buffer, frame.packet.data_len);
        //         printf("Tamanho do buffer: %d\n", frame.packet.data_len);
    }

    if (frame.packet.data_len > 0)
    {
        frame.packet.data = (char *)malloc(frame.packet.data_len);
        memcpy(frame.packet.data, &buffer[DATA_LEN_POS + sizeof(uint)], frame.packet.data_len);
    }
    memcpy(&frame.trailer, &buffer[DATA_LEN_POS + sizeof(uint) + frame.packet.data_len], sizeof(uint));

    return frame;
}

// Variáveis "globais" apenas para este arquivo. Elas ajudam com as implementações abaixo.
static char buffer[1024];
static ssize_t n;
static Frame f;

void wait_for_event(event_type *event, int sd, struct sockaddr_in *other, uint frame_len)
{
    socklen_t other_len;
    while (1)
    {
        n = recvfrom(sd, buffer, frame_len, 0, (struct sockaddr *)other, &other_len);

        if (n == -1 && errno == EFAULT)
        {
            printf("Other error %d\n", errno);
        }

        // Se não deu erro, então chegou um frame
        if (n > -1)
        {
            //             if (n > 11)
            //                 printf("Tamanho do frame recebido: %ld\n", n);
            f = str_to_frame(buffer);
            // reenvie please
            if (f.kind == nak)
            {
                *event = checksum_err;
                return;
            }
            else
            {
                *event = frame_arrival;
                return;
            }
        }
    }
}

void from_physical_layer(Frame *frame)
{
    *frame = f;
}

void to_physical_layer(Frame *s, int sd, struct sockaddr_in *other)
{
    uint msg_len;
    char *msg = frame_to_str(*s, &msg_len);
    //     if (msg_len > 11) {
    //         print_msg("msg: ", msg+HEADER_LEN, msg_len-TRAILER_LEN);
    //         printf("Tamanho da mensagem: %d\n", msg_len);
    //     }
    sendto(sd, msg, msg_len, 0, (struct sockaddr *)other, sizeof(*other));
    free(msg);
}
