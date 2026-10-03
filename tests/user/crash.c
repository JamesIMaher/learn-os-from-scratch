/* crash - a user program that dereferences a NULL pointer. */
#include "ulib.h"

int main(void)
{
    puts("crash-about-to-fault");
    volatile int *p = 0;
    *p = 1;
    puts("crash-SHOULD-NOT-PRINT");
    return 0;
}
