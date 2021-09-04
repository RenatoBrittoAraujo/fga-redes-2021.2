#include <stdint.h>

#include "packets.h"

char **packets_to_file(const char **packets, const int packet_count, const int packet_content_size)
{
    int CHAR_BITS = sizeof(char) * 8;

    int filep = 0;
    int file_capacity = 4;
    char *file_content = (char *)malloc(sizeof(char) * file_capacity);

    for (uint64_t packet_index = 0; packet_index < packet_count; packet_index++)
    {
        /*
           Começa a manipulação dos bits, capaz de tornar pacotes em arquivo.

           NÂO ESTÁ OTIMIZADO, está feito assim para facilitar entendimento do avaliador ( e o meu :) )
        */

        if (file_capacity * CHAR_BITS < filep * CHAR_BITS + packet_content_size)
        {
            file_capacity *= 2;
            file_content = (char *)realloc(file_content, sizeof(char) * file_capacity);
        }

        // Recupera conteúdo do packet
        for (int fi = 0; fi < packet_content_size; fi++)
        {
            int packetbit = (1UL << (fi % CHAR_BITS)) & packets[packet_index][fi / CHAR_BITS];
            if (packetbit)
            {
                file_content[filep / CHAR_BITS] |= (1 << (filep % CHAR_BITS));
            }
            filep++;
        }
    }
    file_content[filep] = '\0';

    // Valores de retorno
    return file_content;
}

void file_to_packets(
    const char *file_content,
    const int packet_content_size,
    char ***packets_o,
    int *packet_count_o)
{
    /*
        Aqui fazemos o processo de dividir a string em seus respectivos quadros e precalcular
        constantes
    */

    int CHAR_BITS = sizeof(char) * 8;

    int file_size = strlen(file_content);  // bytes
    int bit_count = file_size * CHAR_BITS; // bits

    int packet_size = packet_content_size + PACKET_INDEX_BIT_SIZE + PACKET_PARITY_BIT_SIZE; // bits

    int packet_count = 1 + ((bit_count - 1) / packet_content_size); // ceil bit_count / packet_content_size
    int packet_string_size = 1 + ((packet_size - 1) / CHAR_BITS);   // ceil packet_size / bits num char

    int filep = 0;

    char **packets = (char **)malloc(sizeof(char *) * (packet_count + 1));

    for (uint64_t packet_index = 0; packet_index < packet_count; packet_index++)
    {
        /*
           Começa a manipulação dos bits, capaz de gerar quadros de qualquer tamanho e enviando-os
           packet por packet.

           NÂO ESTÁ OTIMIZADO, está feito assim para facilitar entendimento do avaliador ( e o meu :) )
        */

        char *packet = (char *)malloc(sizeof(char) * packet_string_size);
        { // Seta todos os bits do packet para 0
            char zerochar = '?';
            for (int i = 0; i < CHAR_BITS; i++)
            {
                zerochar &= ~(1UL << i);
            }
            for (int i = 0; i < packet_string_size; i++)
            {
                packet[i] = zerochar;
            }
        }

        // Insere conteúdo do packet
        for (int fi = 0; fi < packet_content_size; fi++)
        {
            if (filep >= file_size * 8)
            {
                break;
            }
            int filebit = (1UL << (filep % CHAR_BITS)) & file_content[filep / CHAR_BITS];
            if (filebit)
            {
                packet[fi / CHAR_BITS] |= (1UL << (fi % CHAR_BITS));
            }
            filep++;
        }

        // Insere index do quadro
        for (int fi = packet_content_size; fi < packet_content_size + PACKET_INDEX_BIT_SIZE; fi++)
        {
            int intbit = !!((1LL << (fi - packet_content_size)) & packet_index);
            if (intbit)
            {
                packet[fi / CHAR_BITS] |= (1UL) << (fi % CHAR_BITS);
            }
        }

        int parity_bit = 0; // faz da soma de paridade final ser sempre par
        {
            for (int i = 0; i < packet_content_size + PACKET_INDEX_BIT_SIZE; i++)
            {
                int bitis1 = packet[i / CHAR_BITS] & (1UL << (i % CHAR_BITS));
                if (bitis1)
                {
                    parity_bit = !parity_bit;
                }
            }
        }

        // Insere bit de paridade
        int pos = packet_content_size + PACKET_INDEX_BIT_SIZE;
        if (parity_bit)
        {
            packet[pos / CHAR_BITS] |= (1 << (pos % CHAR_BITS));
        }

        // packet completo :) nossa essa função teve erro demais eu to exausto de debugar kk
        packets[packet_index] = packet;
    }

    // Add end packet
    packets[packet_count] = (char *)malloc(sizeof(char *) * packet_string_size);
    for (int i = 0; i < packet_size; i++)
    {
        packets[packet_count][i / CHAR_BITS] |= 1 << (i % CHAR_BITS);
    }

    // Valores de retorno
    (*packet_count_o) = packet_count + 1;
    (*packets_o) = packets;
}