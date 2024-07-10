#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "SimpleOS.h"
#include <setjmp.h>

#define MAX_ERRBUFF_SIZE 100

typedef int (*TaskFunc_t)(void);

typedef enum {
    KERNEL_RUNNING = 0,
    KERNEL_HALTED,
    KERNEL_NOT_RUNNING
} kernelStatus;

typedef enum {
    TASK_DONE = 0,
    TASK_TRIES_EXPIRED,
    TASK_TIME_EXPIRED,
    TASK_TERMINATED,
    TASK_RETRYING
}TASK_EXIT_CODES;

typedef enum {
    BASIC = 0,
    FAST_RETRYING,  // if function fails (returns 0) it will retry imideatelly until success or run out of tries
    STUBBORN,       // repeats infinelly until success
    BRUTE           // basically fast retrying stubborn
}FUNCTYPES;


typedef struct {
    long TaskCount;

    TaskFunc_t* TaskFunc;
    FUNCTYPES* functype;
    long* TaskTime;
    long* TaskMaxTries;

    int* ERRbuffer;
    long ERRbufferSize;

    kernelStatus status;
    long MaxTasks;

    int kernelRunningFlag;
    int kernelHaltFlag;
} TaskList_struct;

long Task_count(void);
int Task_add(TaskFunc_t func, FUNCTYPES functype, long time, long MaxTries);
int Task_remove(long index);
int Task_execute();
void Task_terminate();

long Task_current_time_left();
void Task_time_subtract();

int kernel_init(long MaxTasksinput);
void kernel_begin();
void kernel_end();
void kernel_halt();
void kernel_continue();
kernelStatus kernel_status();

int kernel_bufferErr(int ERRcode);
int kernel_getErrCode();