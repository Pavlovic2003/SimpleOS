#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "SimpleOS.h"

typedef void (*TaskFunc_t)(void);

typedef struct {
    long TaskCount;
    TaskFunc_t* TaskFunc;
    long* TaskTime;
} TaskList_struct;

typedef enum {
    KERNEL_RUNNING = 0,
    KERNEL_HALTED,
    KERNEL_NOT_RUNNING
} kernelStatus;

long Task_count(void);
int Task_add(TaskFunc_t *func, long time);
int Task_remove(size_t index);
int Task_execute();
long Task_current_time_left();
int Task_time_subtract(long deltaTime);

int kernel_init(long MaxTasksinput);
int kernel_start();
void kernel_terminate();
void kernel_halt();
void kernel_continue();
kernelStatus kernel_status();