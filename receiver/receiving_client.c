#include <stdio.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define MAX 10

void prefix()
{
    printf("[receiving_client] ");
}

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[100];
} message;

int main(int argc, char **argv)
{
    // aguarda e printa...
}