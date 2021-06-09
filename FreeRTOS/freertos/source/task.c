#include "task.h"
/* FreeRTOS中，任务的创建有两种方法，一种是使用动态创建，一直用是使用静态创建。动态创建时，任务
 * 控制块和栈的内存是创建任务时动态分配的，任务删除时，内存可以释放。静态创建时，任务控制块和栈
 * 的内存需要事先定义好，是静态的内存，任务删除时，内存不能释放。目前我们以静态创建为例，
 * configSUPPORT_STATIC_ALLOCATION在FreeOTOSConfig.h中定义，配置为1 */
#if(configSUPPORT_STATIC_ALLOCATION == 1)

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode, const char * const pcName, const uint32_t ulStackDepth, void * const pvParameters, StackType_t * const puxStackBuffer, TCB_t * const pxTaskBuffer)
{
    TCB_t *pxNewTCB;
    /* 定义一个任务句柄xReturn，任务句柄用于指向任务的TCB。任务句柄是数据类型为TaskHandle_t,在task.h中定义，实际上就是一个空指针 */
    TaskHandle_t xReturn;

    if((pxTaskBuffer != NULL) && (puxStackBuffer != NULL))
    {
        pxNewTCB = (TCB_t *) pxTaskBuffer;
        pxNewTCB->pxStack = (StackType_t *) puxStackBuffer;

        /* 创建新的任务，该函数在task.c实现 */
        prvInitialiseNewTask(pxTaskCode,    /* 任务入口 */
                            pcName,         /* 任务名称，字符串形式 */
                            ulStackDepth,   /* 任务栈大小，单位为字 */
                            pvParameters,   /* 任务形参 */
                            &xReturn,       /* 任务句柄 */
                            pxNewTCB)       /* 任务栈起始地址 */
    }else
    {
        xReturn = NULL;
    }
    /* 返回任务句柄，如果任务创建成功，此时xReturn应该指向任务控制块 */
    return xReturn;
}

static void prvInitialiseNewTask(TaskFunction_t pxTaskCode, const char * const pcName, const uint32_t ulStackDepth, void * const pvParameters, TaskHandle_t * const pxCreatedTask, TCB_t * pxNewTCB)
{
    StackType_t *pxTopOfStack;
    UBaseType_t x;

    /* 获取栈地址 */
    pxTopOfStack = pxNewTCB->pxStack + ( ulStackDepth - ( uint32_t ) 1 );
    /* 将栈顶指针向下做8字节对齐，在Cortex-M3（M4或M7）内核的单片机中，因为总线宽度是32位的，通常只要栈保持4字节对
     * 齐就行，可这样为啥要8字节？难道有哪些操作是64位的？确实有，那就是浮点运算，所以要8字节对齐（但是目前我们都还
     * 没有涉及浮点运算，只是为了后续兼容浮点运行的考虑）。如果栈顶指针是8字节对齐的，在进行向下8字节对齐的时候，指
     * 针不会移动，如果不是8字节对齐的，在做向下8字节对齐的时候，就会空出几个字节，不会使用，比如当pxTopOfStack是33
     * ，明显不能整除8，进行向下8字节对齐就是32，那么就会空出一个字节不使用 */
    pxTopOfStack = ( StackType_t * ) ( ( ( uint32_t ) pxTopOfStack ) & ( ~( ( uint32_t ) 0x0007 ) ) );
    /* 将任务的名字存储在TCB中 */
    for( x = ( UBaseType_t ) 0; x < ( UBaseType_t ) configMAX_TASK_NAME_LEN; x++ )
    {
        pxNewTCB->pcTaskName[x] = pcName[x];
        if(pcName[x] == 0x00)
        {
            break;
        }
    }
    /* 任务名字的长度不能超过configMax_TASK_NAME_LEN，并以'0'结尾 */
    pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';
    /* 初始化TCB中的xStatelistItem节点，即初始化该节点所在的链表为空，表示节点还没有插入任何链表 */
    vListInitialiseItem( &( pxNewTCB->xStateListItem ) );
    /* 设置xStateListItem节点的拥有者，即拥有这个节点本身的TCB */
    listSET_LIST_ITEM_OWNER( &( pxNewTCB->xStateListItem ), pxNewTCB );
    /* 调用pxPortInitialiseStack()函数初始化任务栈，并更新栈顶指针，任务第一次运行的环境参数就
     * 存在任务栈中。该函数在port.c（port.c第一次使用需要在freertosportableRVDSARM_CM3（ARM_CM4或者ARM_CM7）
     * 文件夹下面新建然后添加到工程freertos/source这个组文件）中定义 */
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack, pxTaskCode, pvParameters );

    /* 让任务句柄指向任务控制块 */
    if( ( void * ) pxCreatedTask != NULL )
    {
        *pxCreatedTask = ( TaskHandle_t ) pxNewTCB;
    }
}


/* 就绪列表初始化 */
void prvInitialiseTaskLists( void )
{
    UBaseType_t uxPriority;

    for (uxPriority = ( UBaseType_t ) 0U; uxPriority < ( UBaseType_t ) configMAX_PRIORITIES; uxPriority++)
    {
        vListInitialise( &( pxReadyTasksLists[ uxPriority ] ) );
    }
    
}

/* pxCurrentTCB是一个在task.h定义的全局指针，用于指向当前正在
 * 运行或者即将要运行的任务的任务控制块。目前我们还不支持优先级
 * ，则手动指定第一个要运行的任务 */
TaskHandle_t pxCurrentTCB;

/* 调度器启动 */
void vTaskStartScheduler( void )
{
    /* 手动指定第一个运行的任务 */
    pxCurrentTCB =  &Task1TCB;

    /* 启动调度器 */
    if( xPortStartScheduler() != pdFALSE)
    {
        /* 调度器启动成功，则不会返回，即不会来到这里 */
    }
}

/* 选择任务优先级更高的函数，因为目前没有优先级，只能两个任务轮转 */
void vTaskSwitchContext( void )
{
    /* 两个任务轮流切换 */
    if( pxCurrentTCB == &Task1TCB )
    {
        pxCurrentTCB = &Task2TCB;
    }else{
        pxCurrentTCB = &Task1TCB;
    }
}


#endif/* configSUPPORT_STATIC_ALLOCATION */
