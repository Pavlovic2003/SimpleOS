#include "SimpleOS.h"

TaskList_struct* TaskList;

jmp_buf env;

long Task_count(void)
{
    return TaskList->TaskCount;
}

int kernel_init(long MaxTasksinput) {

    TaskList = (TaskList_struct*)malloc(sizeof(TaskList_struct));
    if (TaskList == NULL) {
        perror("Failed to allocate memory whili initializing SimpleOS");
        return 0;
    }

    TaskList->TaskCount = 0;
    TaskList->TaskFunc = NULL;
    TaskList->functype = NULL;
    TaskList->TaskTime = NULL;
    TaskList->TaskMaxTries = NULL;

    TaskList->ERRbufferSize = 0;
    TaskList->ERRbuffer = NULL;

    TaskList->status= KERNEL_NOT_RUNNING;
    TaskList->MaxTasks = MaxTasksinput;

    TaskList->kernelRunningFlag = 0;
    TaskList->kernelHaltFlag = 0;

    return 1;
}

int kernel_bufferErr(int ERRcode)
{
    if (TaskList->ERRbufferSize >= MAX_ERRBUFF_SIZE)
    {
        perror("Error buffer is already full");
        return 0;
    }

    int* newERRbuffer = (int*)realloc(TaskList->ERRbuffer, (TaskList->ERRbufferSize + 1) * sizeof(int));

    if (newERRbuffer == NULL)
    {
        perror("Failed to reallocate memory while buffering error");
        return 0;
    }

    TaskList->ERRbuffer = newERRbuffer;
    TaskList->ERRbuffer[TaskList->ERRbufferSize] = ERRcode;

    TaskList->ERRbufferSize++;

    return 1;
}

int Task_add(TaskFunc_t func, FUNCTYPES functype, long time, long MaxTries) {

    if (TaskList->TaskCount >= TaskList->MaxTasks)
    {
        perror("Task List is already full");
        return 0;
    }

    TaskFunc_t* newFuncs = (TaskFunc_t*)realloc(TaskList->TaskFunc, (TaskList->TaskCount + 1) * sizeof(TaskFunc_t));
    FUNCTYPES* newFuncTypes = (FUNCTYPES*)realloc(TaskList->functype, (TaskList->TaskCount + 1) * sizeof(FUNCTYPES));
    long* newTimes = (long*)realloc(TaskList->TaskTime, (TaskList->TaskCount + 1) * sizeof(long));
    long* newMaxTries = (long*)realloc(TaskList->TaskMaxTries, (TaskList->TaskCount + 1) * sizeof(long));

    if (newFuncs == NULL || newFuncTypes == NULL || newTimes == NULL || newMaxTries == NULL) 
    {
        perror("Failed to reallocate memory while adding task");
        return 0;
    }

    TaskList->TaskFunc = newFuncs;
    TaskList->TaskFunc[TaskList->TaskCount] = func;

    TaskList->functype = newFuncTypes;
    TaskList->functype[TaskList->TaskCount] = functype;

    TaskList->TaskTime = newTimes;
    TaskList->TaskTime[TaskList->TaskCount] = time;

    TaskList->TaskMaxTries = newMaxTries;
    TaskList->TaskMaxTries[TaskList->TaskCount] = MaxTries;

    TaskList->TaskCount++;

    return 1;
}

int Task_remove(long index) 
{
    if (index >= TaskList->TaskCount) {
        fprintf(stderr, "Index out of bounds\n");
        return 0;
    }

    // Shift all elements after the index to the left by one position
    for (long i = index; i < TaskList->TaskCount - 1; ++i) 
    {
        TaskList->TaskFunc[i] = TaskList->TaskFunc[i + 1];
        TaskList->functype[i] = TaskList->functype[i + 1];
        TaskList->TaskTime[i] = TaskList->TaskTime[i + 1];
        TaskList->TaskMaxTries[i] = TaskList->TaskMaxTries[i + 1];
    }

    TaskList->TaskCount -= 1;

    // Handle the case where the task list becomes empty
    if (TaskList->TaskCount == 0) {
        free(TaskList->TaskFunc);
        free(TaskList->functype);
        free(TaskList->TaskTime);
        free(TaskList->TaskMaxTries);
        TaskList->TaskFunc = NULL;
        TaskList->functype = NULL;
        TaskList->TaskTime = NULL;
        TaskList->TaskMaxTries = NULL;
    }
    else {
        // Reallocate memory to shrink the arrays by one element
        TaskFunc_t* newFuncs = (TaskFunc_t*)realloc(TaskList->TaskFunc, TaskList->TaskCount * sizeof(TaskFunc_t));
        FUNCTYPES* newFuncTypes = (FUNCTYPES*)realloc(TaskList->functype, TaskList->TaskCount * sizeof(FUNCTYPES));
        long* newTimes = (long*)realloc(TaskList->TaskTime, TaskList->TaskCount * sizeof(long));
        long* newMaxTries = (long*)realloc(TaskList->TaskMaxTries, TaskList->TaskCount * sizeof(long));

        if (newFuncs == NULL || newTimes == NULL || newMaxTries == NULL)
        {
            perror("Failed to reallocate memory while removing task");
            return 0;
        }

        TaskList->TaskFunc = newFuncs;
        TaskList->functype = newFuncTypes;
        TaskList->TaskTime = newTimes;
        TaskList->TaskMaxTries = newMaxTries;
    }

    return 1;
}

int Task_execute()
{
    if (!TaskList->kernelRunningFlag)   TaskList->status = KERNEL_NOT_RUNNING;
    else if (TaskList->kernelHaltFlag)  TaskList->status = KERNEL_HALTED;
    else                                TaskList->status = KERNEL_RUNNING;

    while (TaskList->TaskMaxTries[0] > 0 || TaskList->functype[0] == STUBBORN || TaskList->functype[0] == BRUTE)
    {
        int exit = setjmp(env); // setjump alows to cancel running function
        if (exit == 0)
        {
            if (TaskList->TaskFunc[0]() == TASK_DONE)
            {
                Task_remove(0); 
                return TASK_DONE;
            }
        
            TaskList->TaskMaxTries[0]--;

            if (TaskList->functype[0] != FAST_RETRYING || TaskList->functype[0] == BRUTE)
            {
                Task_current_to_end();
                return TASK_RETRYING;
            }
        }

        if (exit == TASK_TERMINATED || exit == TASK_TIME_EXPIRED)
        {
            Task_remove(0);
            return exit;
        }
    }
    Task_remove(0);
    return TASK_TRIES_EXPIRED;
}

int Task_current_to_end()
{
    Task_add(TaskList->TaskFunc[0], TaskList->functype[0], TaskList->TaskTime[0], TaskList->TaskMaxTries[0]);
    return Task_remove(0);
}

void Task_terminate()
{
    longjmp(env, TASK_TERMINATED);
}

long Task_current_time_left()
{
    return TaskList->TaskTime[0];
}

void Task_time_subtract()
{
    if (TaskList->TaskTime[0] <= 0) longjmp(env, TASK_TIME_EXPIRED);

    TaskList->TaskTime[0]--;
}

void kernel_begin()
{
    TaskList->kernelRunningFlag = 1;
    while (TaskList->kernelRunningFlag)
       if (TaskList->TaskCount > 0 && !TaskList->kernelHaltFlag) printf("task executed with code: %d\n",Task_execute());
}

kernelStatus kernel_status()
{
    return TaskList->status;
}

void kernel_end()
{
    TaskList->kernelRunningFlag = 0;
}

void kernel_halt()
{
    TaskList->kernelHaltFlag = 1;
}

void kernel_continue()
{
    TaskList->kernelHaltFlag = 0;
}