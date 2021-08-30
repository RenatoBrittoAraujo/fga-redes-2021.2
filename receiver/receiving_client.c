#include <stdio.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define MAX 10

char program_prefix[] = "[receiving_client]";

void prefix()
{
    printf("%s ", program_prefix);
}

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[100];
} message;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        prefix();
        printf("Uso: %s <filepath>\n", argv[0]);
    }
}