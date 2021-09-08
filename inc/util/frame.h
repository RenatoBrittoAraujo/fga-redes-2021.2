#ifndef FRAME_H
#define FRAME_H

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <string.h>
#include <stdlib.h>

/**
 * Tamanho em bytes do segmento do cabeçalho e do Trailer.
 * O tamanho do segmento de dados variável.
 * Observe que o tamanho minímo para um frame é 11 bytes, caso o segmento de dados seja vazio.
 */

#define HEADER_LEN 7
#define TRAILER_LEN 4

#define KIND_POS 0
#define SEQ_POS 1
#define ACK_POS 2
#define DATA_LEN_POS 3

typedef enum
{
    frame_arrival,
    checksum_err
} event_type;
typedef enum
{
    nak,
    data
} kind;
typedef unsigned int uint;

typedef struct
{
    char *data;
    uint data_len;
} Packet;

typedef struct
{
    char kind;
    char seq;
    char ack;
    Packet packet;
    uint trailer;
} Frame;

/**
 * Transforma o frame em string e a retorna. O tamanho do frame é salvo em buffer_len.
 * @arg frame -> Struct do tipo Frame
 * @arg buffer_len -> Ponteiro para um inteiro sem sinal
 * @return -> A versão em string do frame
 */
char *frame_to_str(Frame frame, uint *buffer_len);

/**
 * Transforma a string em um frame.
 * @arg buffer -> frame no formato string
 * @return -> frame criado a partir da string.
 */
Frame str_to_frame(char *buffer);

// Algumas funções úteis. Baseado do livro do Tanenbaum.

/**
 * Espera um evento acontecer e salva o tipo na variável event
 * Neste caso a ideia é esperar o próximo frame do socket.
 */
void wait_for_event(event_type *event, int sd, struct sockaddr_in *other, uint frame_data_len);

/* Recebe um pacote da camada acima. */
void from_network_layer(Packet *packet, mqd_t queue, uint frame_len);

/* Envia um pacote para a camada acima */
void to_network_layer(const Packet *packet, const mqd_t queue);

/* Recebe um pacote da camada física (ou no nosso caso, do socket) */
void from_physical_layer(Frame *r);

/* Envia um pacote para a camada física */
void to_physical_layer(Frame *s, int sd, struct sockaddr_in *other);

#define inc(k, MAX_SEQ) \
    if (k < MAX_SEQ)    \
        k = k + 1;      \
    else                \
        k = 0

#endif
