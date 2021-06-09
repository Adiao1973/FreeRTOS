#include "freertos/source/task.h"

/* 定义任务控制块 */
TCB_t Task1TCB;
TCB_t Task2TCB;
/* 定义任务句柄 */
TaskHandle_t Task1_Handle;
TaskHandle_t Task2_Handle;

/* 软件延迟 */
void delay(uint32_t count)
{
    for(;count!=0;count--);
}

/* 任务1 */
void Task1_Entry(void *p_arg)
{
    int flag1;
    for (;;)
    {
        flag1 = 1;
        delay(100);
        flag1 = 0;
        delay(100);
    }
    
}

/* 任务2 */
void Task2_Entry(void *p_arg)
{
    int flag2;
    for (;;)
    {
        flag2 = 1;
        delay(100);
        flag2 = 0;
        delay(100);
    }
    
}

int main()
{
    /* 将任务插入到就绪列表 */
    /* 初始化与任务相关的列表，如就绪列表 */
    prvInitialiseTaskLists();

    Task1_Handle =                                          /* 任务句柄 */
        xTaskCreateStatic( ( TaskFunction_t ) Task1_Entry,  /* 任务入口 */
                        ( char * ) "Task1",              /* 任务名称，字符串形式 */
                        ( uint32_t ) TASK1_STACK_SIZE,   /* 任务栈大小，单位为字 */
                        ( void * ) NULL,                 /* 任务形参 */
                        ( StackType_t * ) Task1Stack,    /* 任务栈起始地址 */
                        ( TCB_t * ) &Task1TCB);          /* 任务控制块 */

    /* 将任务添加到就绪列表 */
    vListInsertEnd( &( pxReadyTasksLists[1] ), &( ( ( TCB_t * )( &Task1TCB ) )->xStateListItem ) );

    Task2_Handle =                                          /* 任务句柄 */
        xTaskCreateStatic( ( TaskFunction_t ) Task2_Entry,  /* 任务入口 */
                        ( char * ) "Task2",              /* 任务名称，字符串形式 */
                        ( uint32_t ) TASK2_STACK_SIZE,   /* 任务栈大小，单位为字 */
                        ( void * ) NULL,                 /* 任务形参 */
                        ( StackType_t * ) Task2Stack,    /* 任务栈起始地址 */
                        ( TCB_t * ) &Task2TCB);          /* 任务控制块 */

    /* 将任务添加到就绪列表 */
    vListInsertEnd( &( pxReadyTasksLists[2] ), &( ( ( TCB_t * )( &Task2TCB ) )->xStateListItem ) );

}

