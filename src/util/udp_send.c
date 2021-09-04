#ifndef QUEUE_H
#define QUEUE_H

/*
    transmit_packets

    Envia todos os pacotes na ordem que recebe ao host indicado

    Entradas ->
        - packets: pacotes a serem enviados
        - packet_size: tamanho do pacote a ser recebido
        - ip: ip para enviar
        - host: porta para enviar
*/
void transmit_packets(
    const char **packets,
    const int packet_size,
    const char *ip,
    const char *port);

/*
    listen_for_transmission

    Sistema bloqueante que escuta em algum host e coleta pacotes até receber
    um pacote de encerrameto

    Entradas ->
        - packet_size: tamanho do pacote a ser recebido
        - ip: ip para abrir porta e escutar
        - host: porta para abrir e escutar
    Saídas ->
        - return: ponteiro para memória alocada com os pacotes. 
*/
char ***listen_for_transmission(
    const int packet_size,
    const char *ip,
    const char *port);

// int validate_packets(const char* packet)

#endif