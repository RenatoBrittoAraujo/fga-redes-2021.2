#ifndef QUEUE_H
#define QUEUE_H

#define MAX_MSG_BLOCK_SIZE 10

/*
    Elementos da fila de comunicação é composta de 2 itens,
    o primeiro, message_type, é o indice de fila que será consumida/
    enviada. No caso, você pode comunicar de forma fácil entre filas,
    mas cuidado para que os indices não colidam um com os outros. O
    segundo é o texto da mensagem, uma simples string.

    Note que a fila bloqueia o processo até que a transmissão esteja
    concluida.
*/
struct mesg_buffer
{
    long mesg_type;
    char mesg_text[MAX_MSG_BLOCK_SIZE * 2];
};

/*
    get_client_message

    Escuta por mensagens na fila indicada pelo indice. A transmissão é
    divida em partes e durante o processo também envia mensagens ACK
    até que a transmissão esteja completa

    Entradas ->
        - queue_index: indice da fila para receber
    Saídas ->
        - return: ponteiro para string recebida
*/
char *get_client_message(const int queue_index);

/*
    send_message_buffer

    Escuta por mensagens na fila indicada pelo indice. A transmissão é
    divida em partes e durante o processo também recebe mensagens ACK
    até que a transmissão esteja completa

    Entradas ->
        - data: string com dados a serem enviados
        - data_size: tamanho dos dados a ser enviados
        - queue_index: indice da fila para enviar
*/
void send_message_buffer(const char *data, const int data_size, const int queue_index);

#endif