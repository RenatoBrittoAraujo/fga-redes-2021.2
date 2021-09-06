#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

mqd_t init_mq(char* queue_name, int flag, int mq_len, int mq_max_msg);

mqd_t get_mq(char* queue_name, int flag);

struct mq_attr get_mqueue_attr(mqd_t mq);
