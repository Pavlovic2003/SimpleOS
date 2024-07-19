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

enum {
    PRIORITY_LOW = -10,
    PRIORITY_NORMAL = 0,
    PRIORITY_HIGH = 10,
    PRIORITY_URGENT = 1000
};


typedef struct {
    long TaskCount;

    TaskFunc_t* TaskFunc;
    long* TaskTime;
    long* TaskMaxTries;
    int* TaskPriority;

    int* ERRbuffer;
    long ERRbufferSize;

    kernelStatus status;
    long MaxTasks;

    int kernelRunningFlag;
    int kernelHaltFlag;
} TaskList_struct;

long Task_count(void);
int Task_add(TaskFunc_t func, int priority, long time, long MaxTries);
int Task_remove(long index);
int Task_execute();
void Task_terminate();

long Task_current_time_left();
void Task_time_subtract();

void TaskList_sort_priorities();
int compare_priority(const void* a, const void* b);

int kernel_init(long MaxTasksinput);
void kernel_begin();
void kernel_end();
void kernel_halt();
void kernel_continue();
kernelStatus kernel_status();

int kernel_bufferErr(int ERRcode);
int kernel_getErrCode(); 

void print_Tasks();