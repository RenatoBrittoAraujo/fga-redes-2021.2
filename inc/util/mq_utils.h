#ifndef MQ_UTILS_H
#define MQ_UTILS_H

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

mqd_t init_mq(char *queue_name, int flag, int mq_len, int mq_max_msg);

mqd_t get_mq(char *queue_name, int flag);

struct mq_attr get_mqueue_attr(mqd_t mq);

void send_msg_to_queue(mqd_t qd, char *msg, int msg_len, int part_len);

void receive_msg_from_queue(mqd_t qd, char *msg, long *msg_len, struct mq_attr attr);

char *read_file(const char *file_name, int *file_len);

void write_file(const char *file_name, char *file_content, int file_len, char *mode);

#endif
