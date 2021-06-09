#ifndef TASK_H
#define TASK_H
#include "FreeRTOSConfig.h"

/* 任务切换函数 */
#define taskYIELD()     portYIELD()

/* 任务句柄 */
typedef void* TaskHandle_t;

/* 任务控制块 */
typedef struct tskTaskControlBlock
{
    /* 栈顶指针，作为TCB的第一个成员 */
    volatile StackType_t *pxTopOfStack; 
    
    /* 任务节点，这是一个内置在TCB控制块中的链表节点，通过这个节点，可以将任务控制块挂接到各种链表中。这个节点就类似晾衣架的钩子，TCB就是衣服。 */
    ListItem_t xStateListItem; 
    
    /* 任务栈起始地址 */
    StackType_t *pxStack; 
    
    /* 任务名称，字符串形式，长度由宏configMAX_TASK_NAME_LEN来控制，该宏在FreeRTOSConfig.h重定义，默认为16 */
    char pcTaskName[configMAX_TASK_NAME_LEN];

} tskTCB;

/* 数据类型重定义 */
typedef tskTCB TCB_t;

/* 任务创建函数 */
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,           /* 任务入口，既任务的函数名称。TaskFunction_t是在projdef.h
                                                                     * (projdef.h第一次使用需要在include文件夹下面新建然后添加到
                                                                     * 工程freertos/source这个组文件)中重定义的一个数据类型，实际
                                                                     * 就是空指针 */ 
                               const char * const pcName,           /* 任务名称，字符串形式，方便调试 */
                               const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
                               void * const pvParameters,           /* 任务形参 */
                               StackType_t * const puxStackBuffer,  /* 任务栈起始地址 */
                               TCB_t * const pxTaskBuffer);         /* 任务控制块指针 */

/* 创建新的任务 */
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,         /* 任务入口 */
                                 const char * const pcName,         /* 任务名称，字符串形式 */
                                 const uint32_t ulStackDepth,       /* 任务栈大小，单位为字 */
                                 void * const pvParameters,         /* 任务形参 */
                                 TaskHandle_t * const pxCreatedTask,/* 任务句柄 */
                                 TCB_t *pxNewTCB);                  /* 任务控制块指针 */

/* 任务就绪列表,就绪列表实际上就是List_t类型的数组，数组的大小由决定最大任务优先级的宏configMAX_PRIORITIES决定，
 * configMAX_PRIORITIES在FreeRTOSConfig.h中默认定义为5，最大支持256个优先级。数组的下标对应了任务的优先级，同
 * 一优先级的任务统一插入到就绪列表的同一条链表中*/
List_t pxReadyTasksLists[ configMAX_PRIORITIES ];

/* 就绪列表初始化 */
void prvInitialiseTaskLists( void );

/* 调度器启动 */
void vTaskStartScheduler( void );

/* 选择任务优先级更高的函数，因为目前没有优先级，只能两个任务轮转 */
void vTaskSwitchContext( void );


/* 进入临界段 */
#define taskENTER_CRITICAL()            portENTER_CRITICAL()
#define taskENTER_CRITICAL_FROM_ISR()   portSET_INTERRUPT_MASK_FROM_ISR()

/* 退出临界段 */
#define taskEXIT_CRITICAL()             portEXIT_CRITICAL()
#define taskEXIT_CRITICAL_FROM_ISR(x)   portCLEAR_INTERRUPT_MASK_FROM_ISR(x)

#endif /* TASK_H */

