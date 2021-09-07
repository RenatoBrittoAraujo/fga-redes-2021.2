#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>

#include "../../inc/util/mq_utils.h"
#include "../../inc/util/frame.h"

mqd_t init_mq(char* queue_name, int flag, int mq_len, int mq_max_msg) {
//     struct mq_attr fila_attr;
//     fila_attr.mq_maxmsg = mq_len;
//     fila_attr.mq_msgsize = mq_max_msg;

    mqd_t qd = mq_open(queue_name, flag, S_IRWXU, NULL);
	if (qd < 0) {
		printf("Erro na criacao da fila %s\n", queue_name);
		exit(0);
	}

	return qd;
}

mqd_t get_mq(char *queue_name, int flag) {
	mqd_t qd = mq_open(queue_name, O_RDWR);
	if (qd < 0) {
		printf("Erro ao tentar recuperar a fila %s\n", queue_name);
	}

	return qd;
}

struct mq_attr get_mqueue_attr(mqd_t mq) {
    struct mq_attr attr;
    mq_getattr(mq, &attr);
    return attr;
}

void send_msg_to_queue(mqd_t qd, char* msg, int msg_len, int part_len) {
	struct mq_attr attr;
	mq_getattr(qd, &attr);

    char* iter = msg;
    char* end = iter + msg_len;

    if (part_len > msg_len) {
        part_len = msg_len;
    }

    for (; iter < end;) {
        long n = (iter+part_len-iter);
        if (iter + n > end) n = end - iter;

        mq_send(qd, iter, n, 0);
        iter += part_len;
    }
}

void receive_msg_from_queue(mqd_t qd, char* msg, long* msg_len, struct mq_attr attr) {
    unsigned int p;
    int n = mq_receive(qd, msg, attr.mq_msgsize, &p);
    if (n == -1) {
        *msg_len = -1;
        return;
    }
    *msg_len = n;
}

char* read_file(const char* file_name, int* file_len) {
    FILE *fd = fopen(file_name, "r");

    char* file_content;
    fseek(fd, 0, SEEK_END);
    *file_len = ftell(fd);

    file_content = malloc(sizeof(char)*(*file_len)+1);

    rewind(fd);

    fread(file_content, *file_len, sizeof(char), fd);

    fclose(fd);
    return file_content;
}

void write_file(const char* file_name, char *file_content, int file_len, char* mode) {
    FILE *fd = fopen(file_name, mode);

    fwrite(file_content, sizeof(char), file_len, fd);

    fclose(fd);
}

static void print_msg(const char* prefix, char* buffer, int n) {
    printf("%s", prefix);
    for (int i = 0; i < n; i++) {
        printf("%c", buffer[i]);
    }
    printf("\n");
}

void from_network_layer(Packet* packet, mqd_t queue, uint frame_data_len) {
    struct mq_attr attr;
    mq_getattr(queue, &attr);

    static char buffer[10000];
    static int get_new = 1;
    static char* iter = buffer;
    static long last_msg_len;

    uint prio;
    if (get_new) {
        last_msg_len = mq_receive(queue, buffer, attr.mq_msgsize, &prio);
        get_new = 0;
    }

    if (last_msg_len == -1 && errno != EAGAIN) {
        printf("Not EAGAIN\n Err n %d\n", errno);
    }

    if (last_msg_len == -1 && errno == EFAULT) {
        printf("EAGAIN n %d\n", errno);
    }

    if (last_msg_len == -1) {
        // Fila sem mensagens
        packet->data_len = 0;
        get_new = 1;
        return;
    }

    char* end = buffer + last_msg_len;
    if (iter+frame_data_len > end) {
        memcpy(packet->data, iter, end-iter);
        packet->data_len = end-iter;
//         print_msg("Mensagem na fila: ", iter, end-iter);
        get_new = 1;
        iter = buffer;
        return;
    }
    else {
//         print_msg("Mensagem da fila: ", iter, frame_data_len);
//         printf("Tamanho da ultima mensagem: %ld\n", last_msg_len);
        memcpy(packet->data, iter, frame_data_len);
        packet->data_len = frame_data_len;
    }

    iter += (long)frame_data_len;

}

void to_network_layer(const Packet* packet, const mqd_t queue) {
    mq_send(queue, packet->data, packet->data_len, 0);
}
