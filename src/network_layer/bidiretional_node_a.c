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

// #include <sys/ipc.h>
// #include <sys/msg.h>
// #include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <signal.h>

#define MAX_MSG_BLOCK_SIZE 10
#define FRAME_PARITY_BIT_SIZE 1
#define FRAME_INDEX_BIT_SIZE 64

#define MAX_MSG 100
#define DEFAULT 100
#define MIN_MSG 12

#include "../../inc/util/frame.h"
#include "../../inc/util/mq_utils.h"

mqd_t m_queue_r;
mqd_t m_queue_w;

struct mq_attr attr_r;
struct mq_attr attr_w;

char* READ_MQ = "/R_CLIENT_MESSAGE_QUEUE";
char* WRITE_MQ = "/W_CLIENT_MESSAGE_QUEUE";

int frame_len;
int frame_data_len;

void handle_SIGINT(int sig) {
    mq_close(m_queue_r);
    mq_close(m_queue_w);
    mq_unlink(READ_MQ);
    mq_unlink(WRITE_MQ);
    exit(0);
}

int create_socket_descriptor() {
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        printf("Erro ao criar o socket\n");
        exit(1);
    }
    return sd;
}

void configure_addr(char* addr, int port, struct sockaddr_in* end) {
    end->sin_family = AF_INET;
    if (!strcmp("0", addr)) end->sin_addr.s_addr = htonl(INADDR_ANY);
    else end->sin_addr.s_addr = inet_addr(addr);
    end->sin_port = htons(port);
}

int bind_socket(int sd, struct sockaddr_in* end) {
    int rc = bind(sd, (struct sockaddr*) end, sizeof(*end));
    if (rc < 0) {
        printf("Erro ao dar bind no endereco %s:%d", end->sin_addr.s_addr, end->sin_port);
        exit(1);
    }
    return rc;
}

//Retorna o tamanho da PDU
void process_command_line(int argc, char** argv) {
    if (argc < 2) {
        printf("%s <tamanho_PDU>", argv[0]);
        printf("Tamanho da PDU não declarado\n");
        printf("Usando tamanho de PDU padrao: %d\n", DEFAULT);
        frame_len = DEFAULT;
        frame_data_len = frame_len - HEADER_LEN - TRAILER_LEN;
        return;
    }
    if (atoi(argv[1]) <= 0) {
        printf("Tamanho de DPU inválido\nFavor, declarar um número maior que 0\n");
        exit(1);
    }
    frame_len = atoi(argv[1]);
    frame_data_len = frame_len - HEADER_LEN - TRAILER_LEN;
    if (frame_len < MIN_MSG) {
        printf("Tamanho da DPU deve ser no mínimo %d\n", MIN_MSG);
        exit(1);
    }
}

void print_packet(Packet *p) {
    for (uint i = 0; i < p->data_len; i++) {
        printf("%c", p->data[i]);
    }
}


void main_loop(int sd, struct sockaddr_in endServ) {
    /**
     * Loop baseado no Protocolo 4 da camada de enlace do livro do Tanenbaum.
     */

    uint next_frame_to_send;
    uint frame_expected;
    Frame r, s;
    Packet packet;
    packet.data = malloc(sizeof(char)*frame_data_len);
    packet.data_len = 0;
    event_type event;

    next_frame_to_send = 0;
    frame_expected = 0;

    //Recebe mensagem da fila
    from_network_layer(&packet, m_queue_r, frame_data_len);
    s.kind = data;
    s.packet = packet;
    s.seq = next_frame_to_send;
    s.ack = 1 - frame_expected;
    to_physical_layer(&s, sd, &endServ);

    while (1) {
        wait_for_event(&event, sd, &endServ, frame_data_len);
        if (event == frame_arrival) {
            from_physical_layer(&r);
            if (r.seq == frame_expected) {
                to_network_layer(&r.packet, m_queue_w);
                inc(frame_expected, 1);
            }
            if (r.ack == next_frame_to_send) {
                from_network_layer(&packet, m_queue_r, frame_data_len);
                inc(next_frame_to_send, 1);
            }
        }
        s.kind = data;
        s.packet = packet;
        s.seq = next_frame_to_send;
        s.ack = 1-frame_expected;
        to_physical_layer(&s, sd, &endServ);
    }
}

int main(int argc, char** argv) {

    signal(SIGINT, &handle_SIGINT);

    process_command_line(argc, argv);

    //Pega referência das filas
    m_queue_r = init_mq(READ_MQ, O_RDWR | O_CREAT | O_NONBLOCK, 0, 0);
    m_queue_w = init_mq(WRITE_MQ, O_RDWR | O_CREAT | O_NONBLOCK, 0, 0);

    int sd, rc, i;

    struct sockaddr_in endCli;
    struct sockaddr_in endServ;

    // configura informacoes do servidor
    configure_addr("127.0.0.1", 8080, &endServ);

    //configura informacoes do cliente
    configure_addr("0", htons(0), &endCli);
    sd = create_socket_descriptor();

    // torna as chamadas do socket não bloqueáveis.
    int flags = fcntl(sd, F_GETFL);
    fcntl(sd, F_SETFL, flags | O_NONBLOCK);

    rc = bind_socket(sd, &endCli);

	mq_getattr(m_queue_r, &attr_r);
    mq_getattr(m_queue_w, &attr_w);

    //Código de envio a partir daqui
    main_loop(sd, endServ);

    return 0;
}

