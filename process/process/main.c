#include <stdio.h>
#include <stdlib.h>
#include "SimpleOS.h"


int A(void)
{
    printf("A\n");
    return 1;
}

int B(void)
{
    printf("B\n");
   // while (1) {};
    return 1;
}

int main()
{
    if (!kernel_init(100)) {
        fprintf(stderr, "Task_init failed\n");
        return 1;
    }

    Task_add(A, BASIC, 10, 5);
    Task_add(B, FAST_RETRYING, 30, 4);

    printf("Task count: %d\n", Task_count());

    kernel_begin();

	return 0;
}



