#ifndef QUEUE_H
#define QUEUE_H

#define FRAME_PARITY_BIT_SIZE 1
#define FRAME_INDEX_BIT_SIZE 64

/*
    packets_to_file

    Transforma pacotes em string (arquivo)

    Entradas ->
        - packet_size: tamanho do pacote a ser recebido
        - ip: ip para abrir porta e escutar
        - host: porta para abrir e escutar
    Saídas ->
        - return: ponteiro para memória alocada com o arquivo. 
*/
char **packets_to_file(
    char **packets,
    int packet_count,
    int packet_content_size);

/*
    file_to_packets

    Transforma string de arquivo em pacotes

    Entradas ->
        - packet_size: tamanho do pacote a ser recebido
        - ip: ip para abrir porta e escutar
        - host: porta para abrir e escutar
    Saídas ->
        - packets_o: ponteiro para memória alocada com os pacotes. 
        - packet_count_o: ponteiro para memória alocada com o número de 
          pacotes.
*/
void file_to_packets(
    char *file_content,
    int packet_content_size,
    char ***packets_o,
    int *packet_count_o);

#endif