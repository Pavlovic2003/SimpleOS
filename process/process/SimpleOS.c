#include "SimpleOS.h"

TaskList_struct* TaskList;

jmp_buf env;

static int* global_priorities;

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
    TaskList->TaskTime = NULL;
    TaskList->TaskMaxTries = NULL;
    TaskList->TaskPriority = NULL;

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

int Task_add(TaskFunc_t func, int priority, long time, long MaxTries) 
{
    if (TaskList->TaskCount >= TaskList->MaxTasks)
    {
        perror("Task List is already full");
        return 0;
    }

    TaskFunc_t* newFuncs = (TaskFunc_t*)realloc(TaskList->TaskFunc, (TaskList->TaskCount + 1) * sizeof(TaskFunc_t));
    long* newTimes = (long*)realloc(TaskList->TaskTime, (TaskList->TaskCount + 1) * sizeof(long));
    long* newMaxTries = (long*)realloc(TaskList->TaskMaxTries, (TaskList->TaskCount + 1) * sizeof(long));
    int* newPriorities = (int*)realloc(TaskList->TaskPriority, (TaskList->TaskCount + 1) * sizeof(int));

    if (newFuncs == NULL || newTimes == NULL || newMaxTries == NULL || newPriorities == NULL)
    {
        perror("Failed to reallocate memory while adding task");
        return 0;
    }

    TaskList->TaskFunc = newFuncs;
    TaskList->TaskFunc[TaskList->TaskCount] = func;

    TaskList->TaskTime = newTimes;
    TaskList->TaskTime[TaskList->TaskCount] = time;

    TaskList->TaskMaxTries = newMaxTries;
    TaskList->TaskMaxTries[TaskList->TaskCount] = MaxTries;

    TaskList->TaskPriority = newPriorities;
    TaskList->TaskPriority[TaskList->TaskCount] = priority;

    TaskList->TaskCount++;

    return 1;
}

// Comparator function for qsort
int compare_priority(const void* a, const void* b) {
    int index_a = *(const int*)a;
    int index_b = *(const int*)b;
    return global_priorities[index_b] - global_priorities[index_a];
}

void TaskList_sort_priorities()
{
    // Create an array of indices
    int* indices = (int*)malloc(TaskList->TaskCount * sizeof(int));
    for (long i = 0; i < TaskList->TaskCount; ++i) {
        indices[i] = i;
    }

    // Set the global priorities pointer to the TaskPriority array
    global_priorities = TaskList->TaskPriority;

    // Sort indices based on TaskPriority
    qsort(indices, TaskList->TaskCount, sizeof(int), compare_priority);

    // Allocate temporary arrays for sorted elements
    TaskFunc_t* sorted_TaskFunc = (TaskFunc_t*)malloc(TaskList->TaskCount * sizeof(TaskFunc_t));
    long* sorted_TaskTime = (long*)malloc(TaskList->TaskCount * sizeof(long));
    long* sorted_TaskMaxTries = (long*)malloc(TaskList->TaskCount * sizeof(long));
    int* sorted_TaskPriority = (int*)malloc(TaskList->TaskCount * sizeof(int));

    // Populate sorted arrays
    for (long i = 0; i < TaskList->TaskCount; i++) {
        sorted_TaskFunc[i] = TaskList->TaskFunc[indices[i]];
        sorted_TaskTime[i] = TaskList->TaskTime[indices[i]];
        sorted_TaskMaxTries[i] = TaskList->TaskMaxTries[indices[i]];
        sorted_TaskPriority[i] = TaskList->TaskPriority[indices[i]];
    }

    // Copy sorted arrays back to the original task list
    for (long i = 0; i < TaskList->TaskCount; i++) {
        TaskList->TaskFunc[i] = sorted_TaskFunc[i];
        TaskList->TaskTime[i] = sorted_TaskTime[i];
        TaskList->TaskMaxTries[i] = sorted_TaskMaxTries[i];
        TaskList->TaskPriority[i] = sorted_TaskPriority[i];
    }

    // Free temporary arrays and indices
    free(sorted_TaskFunc);
    free(sorted_TaskTime);
    free(sorted_TaskMaxTries);
    free(sorted_TaskPriority);
    free(indices);
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
        TaskList->TaskPriority[i] = TaskList->TaskPriority[i + 1];
        TaskList->TaskTime[i] = TaskList->TaskTime[i + 1];
        TaskList->TaskMaxTries[i] = TaskList->TaskMaxTries[i + 1];
    }

    TaskList->TaskCount -= 1;

    // Handle the case where the task list becomes empty
    if (TaskList->TaskCount == 0) {
        free(TaskList->TaskFunc);
        free(TaskList->TaskPriority);
        free(TaskList->TaskTime);
        free(TaskList->TaskMaxTries);

        TaskList->TaskFunc = NULL;
        TaskList->TaskPriority = NULL;
        TaskList->TaskTime = NULL;
        TaskList->TaskMaxTries = NULL;
    }
    else {
        // Reallocate memory to shrink the arrays by one element
        TaskFunc_t* newFuncs = (TaskFunc_t*)realloc(TaskList->TaskFunc, TaskList->TaskCount * sizeof(TaskFunc_t));
        int* newPriorities = (long*)realloc(TaskList->TaskPriority, TaskList->TaskCount * sizeof(int));
        long* newTimes = (long*)realloc(TaskList->TaskTime, TaskList->TaskCount * sizeof(long));
        long* newMaxTries = (long*)realloc(TaskList->TaskMaxTries, TaskList->TaskCount * sizeof(long));

        if (newFuncs == NULL || newTimes == NULL || newMaxTries == NULL || newPriorities == NULL)
        {
            perror("Failed to reallocate memory while removing task");
            return 0;
        }

        TaskList->TaskFunc = newFuncs;
        TaskList->TaskPriority = newPriorities;
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

        int exit = setjmp(env); // setjump alows to cancel running function
        if (exit == 0)
        {

            if (TaskList->TaskFunc[0]() == TASK_DONE)
            {
                Task_remove(0); 
                return TASK_DONE;
            }
            else
            {
                TaskList->TaskMaxTries[0]--;

                if (TaskList->TaskMaxTries[0] > 0)
                {
                    Task_current_to_end();
                    return TASK_RETRYING;
                }      
                else
                {
                    Task_remove(0);
                    return TASK_TRIES_EXPIRED;
                }
            }
        }
        else
        {
            Task_remove(0);
            return exit;
        }
}

int Task_current_to_end()
{
    Task_add(TaskList->TaskFunc[0], TaskList->TaskPriority[0], TaskList->TaskTime[0], TaskList->TaskMaxTries[0]);
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
        if (TaskList->TaskCount > 0 && !TaskList->kernelHaltFlag)
        {
            TaskList_sort_priorities();
            print_Tasks();
            printf("task executed with code: %d\n", Task_execute());
            printf("\n");
        }
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

void print_Tasks()
{
    for (int i = 0; i < TaskList->TaskCount; i++) printf("index: %d\tpriority: %d\tmaxTime: %d\ttriesLeft: %d\n", i, TaskList->TaskPriority[i], TaskList->TaskTime[i], TaskList->TaskMaxTries[i]);
}