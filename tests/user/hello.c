/* hello - the first real user program (lessons 13-15 checkpoints). */
#include "ulib.h"

int main(void)
{
    printf("Hello, user space! pid=%d\n", getpid());
    return 7;
}
