#include <netinet/in.h>
#include <stdio.h>

#include "udp_send.h"

void transmit_packets(
    const char **packets,
    const int packet_count,
    const int packet_size,
    const char *ip,
    const char *port)
{
    int sd, rc, i;
    struct sockaddr_in ladoCli;
    struct sockaddr_in ladoServ;

    ladoServ.sin_family = AF_INET;
    ladoServ.sin_addr.s_addr = inet_addr(ip);
    ladoServ.sin_port = htons(atoi(port));

    ladoCli.sin_family = AF_INET;
    ladoCli.sin_addr.s_addr = htonl(INADDR_ANY);
    ladoCli.sin_port = htons(0);

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        prefix();
        printf("transmit_packets: não pode abrir o socket\n");
        exit(1);
    }

    rc = bind(sd, (struct sockaddr *)&ladoCli, sizeof(ladoCli));
    if (rc < 0)
    {
        prefix();
        printf("transmit_packets: não pode fazer um bind da porta\n");
        exit(1);
    }
    for (int i = 0; i < packet_count; i++)
    {
        char v[10];
        rc = sendto(sd, packets[i], strlen(packets[i]), 0, (struct sockaddr *)&ladoServ, sizeof(ladoServ));
        if (rc < 0)
        {
            prefix();
            printf("transmit_packets: nao pode enviar dados %d\n", i - 1);
            close(sd);
            exit(1);
        }
    }
}

char ***listen_for_transmission(
    const int packet_size,
    const char *ip,
    const char *port)
{
    int sd, rc, n, tam_Cli;
    struct sockaddr_in endCli;
    struct sockaddr_in endServ;
    char msg[MAX_MSG];

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        prefix();
        printf("listen_for_transmission: nao pode abrir o socket \n");
        exit(1);
    }

    endServ.sin_family = AF_INET;
    endServ.sin_addr.s_addr = inet_addr(port);
    endServ.sin_port = htons(atoi(port));

    rc = bind(sd, (struct sockaddr *)&endServ, sizeof(endServ));
    if (rc < 0)
    {
        prefix();
        printf("listen_for_transmission: nao pode fazer bind na porta %s\n", port);
        exit(1);
    }

    prefix();
    printf("listen_for_transmission: esperando por dados no IP: %s, porta UDP numero: %s\n", ip, port);
    int frame_count = 0;
    int frame_capacity = 2;
    char **frames = (char **)malloc(sizeof(char *) * frame_capacity);

    while (1)
    {
        memset(msg, 0x0, MAX_MSG);
        tam_Cli = sizeof(endCli);
        n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *)&endCli, &tam_Cli);
        if (n < 0)
        {
            prefix();
            printf("listen_for_transmission: nao pode receber dados\n");
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
}

// A implementar
// int validate_packets(const char* packet)
