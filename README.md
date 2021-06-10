# FreeRTOS

Self learning FreeRTOS

## 1. FreeRTOS中链表的实现

FreeRTOSS中与链表相关的操作均在list.h和list.c这两个文件中实现，list.h第一次使用需要在include文件夹下面新建然后添加到工程
freertos/source这个组文件，list.c第一次使用需要在freertos文件夹下面新建然后添加到工程freetos/source这个组文件。

### 1.1 实现链表节点

#### 1.1.1 定义链表节点数据结构

* 链表节点的数据结构在list.h中定义，具体实现和节点示意图如下

> ```c
> struct xLIST_ITEM
> {
>    TickType_t xItemValue; /* 辅助值，用于帮助节点做顺序排序*/ (1)
>    struct xLIST_ITEM * pxNext; /* 指向链表下一个节点 */ (2)
>    struct xLIST_ITEM * pxPrevious; /* 指向链表前一个节点 */ (3)
>    void * pvOwner; /* 指向拥有该节点的内核对象，通常是TCB */ (4)
>    void * pvContainner; /* 指向该节点所在的链表 */ (5)
> };
> typedefstruct xLIST_ITEM ListItem_t
> ```

<div align=center>
<img src="https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa008.png">
</div>

* (1):一个辅助值，用于帮助节点做顺序排列。该辅助值的数据类型为TickType_t，在FreeRTOS中，凡是涉及数据类型的地方，FreeRTOS都会将标准的C数据类型用typedef 重新取一个类型名。这些经过重定义的数据类型放在 portmacro.h（portmacro.h第一次使用需要在include文件夹下面新建然后添加到工程freertos/source这个组文件）这个头文件,具体代码如下

> ```c
> #ifndef PORTMACRO_H
> #define PORTMACRO_H
>
> #include"stdint.h"
> #include"stddef.h"
>
> /* 数据类型重定义 */
> #define portCHAR            char
> #define portFLOAT           float
> #define portDOUBLE          double
> #define portLONG            long
> #define portSHORT           short
> #define portSTACK_TYPE      uint32_t
> #define portBASE_TYPE       long
>
> typedef portSTACK_TYPE StackType_t;
> typedeflong BaseType_t;
> typedefunsigned long UBaseType_t;
>
> #if( configUSE_16_BIT_TICKS == 1 )(1)
> typedefuint16_t TickType_t;
> #define portMAX_DELAY ( TickType_t ) 0xffff
> #else
> typedefuint32_t TickType_t;
> #define portMAX_DELAY ( TickType_t ) 0xffffffffUL
> #endif
>
> #endif/* PORTMACRO_H */
> ```

* TickType_t具体表示16位还是32位，由configUSE_16_BIT_TICKS这个宏决定，当该宏定义为1时，TickType_t为16位，否则为32位。该宏在FreeRTOSConfig.h中默认定义为0，具体实现如下，所以TickType_t表示32位。

> ```c
> #ifndef FREERTOS_CONFIG_H
> #define FREERTOS_CONFIG_H
> 
> #define configUSE_16_BIT_TICKS              0
>
> #endif /* FREERTOS_CONFIG_H */
> ```

#### 1.1.2 链表节点初始化

* 链表节点初始化在list.c中实现，具体如下

> ```c
> /* 链表节点初始化 */
> void vListInitialiseItem( ListItem_t * const pxItem)
> {
>   /* 初始化该节点所在的链表为空，表示节点还没有插入任何链表 */
>    pxItem->pvContainner = NULL; 
> }
> ```

* 链表节点ListItem_t总共有5个成员，但是初始化的时候只需将pvContainer初始化为空即可，表示该节点还没有插入到任何链表。一个初始化好的节点示意图具体见下图

![ListItem_t](https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa009.png)

### 1.2 实现链表根节点

#### 1.2.1 定义链表根节点数据结构

* 链表根节点的数据结构在list.h中定义，具体代码如下，根节点示意图也如下

> ```c
> /* 链表根节点数据结构定义 */
> struct xLIST
> {
>     UBaseType_t uxNumberOfItems; /* 链表节点计数器 */
>     ListItem_t * pxIndex; /* 链表节点索引指针 */
>     MiniListItem_t xListEnd; /* 链表最后一个节点 */
> };
> 
> typedef struct xLIST List_t;
> ```

![xLIST](https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa010.png)

* MiniListItem_t链表精简节点结构体定义如下

> ```c
> /* 链表精简节点结构体定义 */
> struct xMINI_LIST_ITEM
> {
>     TickType_t xItemValue; /* 辅助值，用于帮助节点做升序排序 */ 
>     struct xLIST_ITEM * pxNext; /* 指向链表下一个节点 */
>     struct xLIST_ITEM * pxPrevious; /* 指向链表前一个节点 */
> };
> typedef struct xMINI_LIST_ITEM MiniListItem_t; /* 精简节点数据类型重定义 */
> ```

#### 1.2.2 链表根节点初始化

* 链表节点初始化函数在list.c中实现，具体实现如下，初始好的根节点示意图具体如下

> ```c
> /* 链表根节点初始化 */
> void vListInitialise(List_t * const pxList)
> {
>     /* 将链表索引指针指向最后一个节点 */
>     pxList->pxIndex = (ListItem_t *) &(pxList->xListEnd);
>
>     /* 将链表最后一个节点的辅助排序的值设置为最大，确保该节点就是链表的最后节点 */
>     pxList->xListEnd.xItemValue = portMAX_DELAY;
>
>     /* 将最后一个节点的pxNext和pxPrevious指针均指向节点自身，表示链表为空 */
>     pxList->xListEnd.pxNext = (ListItem_t *) &(pxList->xListEnd);
>     pxList->xListEnd.pxPrevious = (ListItem_t *) &(pxList->xListEnd);
> 
>     /* 初始化链表节点计数器的值为0，表示链表为空 */
>     pxList->uxNumberOfItems = (UBaseType_t) 0U;
> }
> ```

![xLIST_init](https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa011.png)

#### 1.2.3 将节点插入到链表的尾部

* 将节点插入到链表的尾部(可以理解为头部)就是将一个新的节点插入到一个空的链表，具体代码实现如下，插入过程如下图所示

> ```c
> /* 将节点接入到链表的尾部 */
> void vListInsertEnd(List_t * const pxList,ListItem_t * const pxNewListItem)
> {
>     ListItem_t * const pxIndex = pxList->pxIndex;
>
>     pxNewListItem->pxNext = pxIndex;
>     pxNewListItem->pxPrevious = pxIndex->pxPrevious;
>     pxIndex->pxPrevious->pxNext = pxNewListItem;
>     pxIndex->pxPrevious = pxNewListItem;
>
>     /* 记住该节点所在的链表 */
>     pxNewListItem->pvContainner = (void *) pxList;
>
>     /* 链表节点计数器++ */
>     (pxList->uxNumberOfItems)++;
> }
> ```

![vListInsertEnd](https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa012.png)

#### 1.2.4 将节点按照升序排列插入到链表

* 将节点按照升序排列插入到链表，如果有两个节点的值相同，则新节点在旧节点的后面插入，具体实现如下

> ```c
> /* 将节点按照升序排列插入到链表 */
> void vListInsert(List_t * const pxList, ListItem_t * const pxNewListItem)
> {
>     /* 遍历节点 */
>     ListItem_t *pxIterator;
>
>     /* 获取节点的排序辅助值 */
>     const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;
>
>     /* 寻找节点要插入的位置 */
>     if(xValueOfInsertion == portMax_DELAY)
>     {
>         pxIterator = pxList->xListEnd.pxPrevious;
>     }
>     else
>     {
>         for (pxIterator = (ListItem_t *) &(pxList->xListEnd); pxIterator->pxNext->xItemValue <= xValueOfInsertion; pxIterator = pxIterator->pxNext)
>         {
>             /* 没有事情可做，不断迭代只为了找到节点要插入的位置 */
>         }
>         
>         /* 根据升序排列，将节点插入 */
>         pxNewListItem->pxNext = pxIterator->pxNext;
>         pxNewListItem->pxNext->pxPrevious = pxNewListItem;
>         pxNewListItem->pxPrevious = pxIterator;
>         pxIterator->pxNext = pxNewListItem;
>
>        /* 记住该节点所在的链表 */
>        pxNewListItem->pvContainner = (void *) pxList;
>
>        /* 链表节点计数器++ */
>        (pxList->uxNumberOfItems)++;
>
>    }
> }
> ```

![vListInsert](https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa013.png)

#### 1.2.5 将节点从链表删除

* 将节点从链表删除具体实现如下。假设将一个有三个节点的链表中的中间节点删除，删除操作的过程示意图如下

> ```c
> /* 将节点从链表删除 */
> UBaseType_t uxListRemove(ListItem_t * const pxItemToRemove)
> {
>     /* 获取节点所在的链表 */
>     List_t * const pxList = (List_t *) pxItemToRemove->pvContainner;
>     /* 将指定的节点从链表里删除 */
>     pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
>     pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
>
>     /* 调整链表的节点索引指针 */
>     if(pxList->pxIndex == pxItemToRemove){
>         pxList->pxIndex = pxItemToRemove->pxPrevious;
>     }
>
>     /* 初始化该节点所在的链表为空，表示节点还没有插入任何链表 */
>     pxItemToRemove->pvContainner = NULL;
>
>     /* 链表节点计数器-- */
>     (pxList->uxNumberOfItems)--;
>
>     /* 返回链表中剩余节点的个数 */
>     return pxList->uxNumberOfItems;
> }
> ```

![uxListRemove](https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa014.png)

#### 1.2.6 节点带参宏小函数

* 在list.h中，还定义了各种各样的带参宏，方便对节点做一些简单的操作，具体实现如下

> ```c
> /* 初始化节点的拥有者 */
> #define listSET_LIST_ITEM_OWNER(pxListItem, pxOwner)\
>     ((pxListItem)->pvOwner = (void*)(pxOwner))
> 
> /* 获取节点拥有者 */
> #define listGET_LIST_ITEM_OWNER(pxListItem)\
>     ((pxListItem)->pvOwner)
>
> /* 初始化节点排序辅助值 */
> #define listSET_LIST_ITEM_VALUE(pxListItem, xValue)\
>     ((pxListItem)->xItemValue = (xValue))
> 
> /* 获取节点排序辅助值 */
> #define listGET_ITEM_VALUE_VALUE(pxListItem)\
>     ((pxListItem)->xItemValue)
> 
> /* 获取链表根节点的节点计数器的值 */
> #define listGET_ITEM_VALUE_OF_HEAD_ENIRY(pxList)\
>     (((pxList)->xListEnd).pxNext->xItemValue)
>
> /* 获取链表的入口节点 */
> #define listGET_HEAD_ENIRY(pxList)\
>     (((pxList)->xListEnd).pxNext)
>
> /* 获取节点的下一个节点 */
> #define listGET_NEXT(pxListItem)\
>     ((pxListItem)->pxNext)
>
> /* 获取链表的最后一个节点 */
> #define listGET_END_MARKER(pxList)\
>     ((ListItem_t const *)(&((pxList)->xListEnd)))
> 
> /* 判断链表是否为空 */
> #define listLIST_IS_EMPTY(pxList)\
>     ((BaseType_t)((pxList)->unxNumberOfItems == (UBaseType_t)))
>
> /* 获取链表的节点数 */
> #define listCURRENT_LIST_LENGTH(pxList)\
>     ((pxList)->uxNumberOfItems)
>
>
> /* 获取链表下一个节点的OWNER,即TCB */
> #define listGET_OWNER_OF_NEXT_ENIRY(pxTCB, pxList)                              \
> {                                                                               \
>     List_t * const pxConstList = (pxList);                                      \
>     /* 节点索引指向链表第一个节点 */                                               \
>     (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;                    \
>     /* */                                                                       \
>     if((void *)(pxConstList)->pxIndex == (void *)&((pxConstList)->xListEnd))    \
>     {                                                                           \
>         (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;                \
>     }                                                                           \
>     /* 获取节点的OWNER，即TCB */                                                 \
>     (pxTCB) = (pxConstList)->pxIndex->pvOwner;                                  \
> }
> ```

## 2. 任务的定义和任务切换的实现

### 2.1 创建任务

#### 2.1.1 定义任务栈

* 在一个裸机系统中，如果有全局变量，有子函数调用，有中断发生。那么系统在运行的时候，全局变量放在哪里，子函数调用时，局部变量放在哪里，中断发生时，函数返回地址放在哪里。如果只是单纯的裸机编程，它们放在哪里我们不用管，但是如果要写一个RTOS，这些种种环境参数，我们必须弄清楚他们是如何存储的。在裸机系统中，他们统统放在一个叫栈的地方，栈是单片机RAM里面一段连续的内存空间，栈的大小一般在启动文件或者链接脚本里面指定，最后由C库函数_main进行初始化

* 但是，在多任务系统中，每个任务都是独立的，互不干扰的，所以要为每个任务都分配独立的栈空间，这个栈空间通常是一个预先定义好的全局数组，也可以是动态分配的一段内存空间，但它们都存在于RAM中

* 现在，我们要实现两个变量按照一定的频率轮流的翻转，每个变量对应一个任务，那么就需要定义两个任务栈。在多任务系统中，有多少个任务就需要定义多少个任务栈。

> ```c
> /* 定义任务栈 */
> #define TASK1_STACK_SIZE    128
> StackType_t Task1Stack[TASK1_STACK_SIZE];
>
> #define TASK2_STACK_SIZE    128
> StackType_t Task2Stack[TASK2_STACK_SIZE];
> ```

* 任务栈其实就是一个预先定义好的全局数据，数据类型为StackType_t,大小由TASK1_STACK_SIZE和TASK2_STACK_SIZE这个宏定义，默认为128，单位为字，即512字节，这也是FreeRTOS推荐的最小任务栈。在FreeRTOS中，凡是涉及数据类型的地方，FreeRTOS都会将标准的C数据类型用typedef重新取一个类型名。这些经过重定义的数据类型放在portmacro.h这个头文件，下面是任务的定义和任务切换的实现都需要用到的

> ```c
> #ifndef PORTMACRO_H
>
> #include "FreeRTOSConfig.h"
> /* 包含标准库头文件 */
> #include "stdint.h"
> #include "stddef.h"
>
> /* 数据类型重定义 */
> #define portCHAR            char
> #define portFLOAT           float
> #define portDOUBLE          double
> #define portLONG            long
> #define portSHORT           short
> #define portSTACK_TYPE      uint32_t
> #define portBASE_TYPE       long
> #endif/* PORTMACRO_H */
> ```

#### 2.1.2 定义任务函数

* 任务是一个独立的函数，函数主体无限循环且不能返回。我们在main.c中定义的两个任务具体如下

> ```c
> /* 软件延时 */
> void delay (uint32_t count)
> {
> for (; count!=0; count--);
> }
> /* 任务1 */
> void Task1_Entry( void *p_arg )(1)
> {
> for ( ;; )
>     {
>         flag1 = 1;
>         delay( 100 );
>         flag1 = 0;
>         delay( 100 );
>     }
> }
> 
> /* 任务2 */
> void Task2_Entry( void *p_arg )(2)
> {
> for ( ;; )
>     {
>         flag2 = 1;
>         delay( 100 );
>         flag2 = 0;
>         delay( 100 );
>     }
> }
> ```

#### 2.1.3 定义任务控制块

* 在裸机系统中，程序的主体是CPU按照顺序执行的。而在多任务系统中，任务的执行是由系统调度的。系统为了顺利的调度任务，为每个任务都额外定义了一个任务控制块，这个任务控制块就相当于任务的身份证，里面存有任务的所有信息，比如任务的栈指针，任务名称，任务的形参等。有了这个任务控制块之后，以后系统对任务的全部操作都可以通过这个任务控制块来实现。定义一个任务控制块需要一个新的数据类型，该数据类型在task.h这个头文件中声明，具体如下，使用它可以为每个任务都定义一个任务控制块实体

> ```c
> /* 任务控制块 */
> typedef struct tskTaskControlBlock
> {
>     /* 栈顶指针，作为TCB的第一个成员 */
>     volatile StackType_t *pxTopOfStack; 
>    
>     /* 任务节点，这是一个内置在TCB控制块中的链表节点，通过这个节点，可以将任务控制块挂接到各种链表中。这个节点就类似晾衣架的钩子，TCB就是衣服。 */
>     ListItem_t xStateListItem; 
>    
>     /* 任务栈起始地址 */
>     StackType_t *pxStack; 
>     
>     /* 任务名称，字符串形式，长度由宏configMAX_TASK_NAME_LEN来控制，该宏在FreeRTOSConfig.h重定义，默认为16 */
>     char pcTaskName[configMAX_TASK_NAME_LEN];
> 
> } tskTCB;
>
> /* 数据类型重定义 */
> typedef tskTCB TCB_t;
> ```

#### 2.1.4 实现任务创建函数

* 任务的栈，任务的函数实体，任务的控制块最终需要联系起来才能由系统进行统一调度。那么这个联系的工作就由任务创建函数xTaskCreateStatic()来实现，该函数在task.c中定义，在task.h中声明，所有跟任务相关的函数都在这个文件定义。xTaskCreateStatic()函数的实现如下

> ```c
> #if(configSUPPORT_STATIC_ALLOCATION == 1)
>
> /* 任务创建函数 */
> TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,           /* 任务入口，既任务的函数名称。TaskFunction_t是在projdef.h
>                                                                      * (projdef.h第一次使用需要在include文件夹下面新建然后添加到
>                                                                      * 工程freertos/source这个组文件)中重定义的一个数据类型，实际
>                                                                      * 就是空指针 */ 
>                                const char * const pcName,           /* 任务名称，字符串形式，方便调试 */
>                                const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
>                                void * const pvParameters,           /* 任务形参 */
>                                StackType_t * const puxStackBuffer,  /* 任务栈起始地址 */
>                                TCB_t * const pxTaskBuffer)          /* 任务控制块指针 */
> {
>     TCB_t *pxNewTCB;
>     /* 定义一个任务句柄xReturn，任务句柄用于指向任务的TCB。任务句柄是数据类型为TaskHandle_t,在task.h中定义，实际上就是一个空指针 */
>     TaskHandle_t xReturn;
> 
>     if((pxTaskBuffer != NULL) && (puxStackBuffer != NULL))
>     {
>         pxNewTCB = (TCB_t *) pxTaskBuffer;
>         pxNewTCB->pxStack = (StackType_t *) puxStackBuffer;
> 
>         /* 创建新的任务，该函数在task.c实现 */
>         prvInitialiseNewTask(pxTaskCode,    /* 任务入口 */
>                             pcName,         /* 任务名称，字符串形式 */
>                             ulStackDepth,   /* 任务栈大小，单位为字 */
>                             pvParameters,   /* 任务形参 */
>                             &xReturn,       /* 任务句柄 */
>                             pxNewTCB)       /* 任务栈起始地址 */
>     }else
>     {
>         xReturn = NULL;
>     }
>     /* 返回任务句柄，如果任务创建成功，此时xReturn应该指向任务控制块 */
>     return xReturn;
> }
> ```

* FreeRTOS中，任务的创建有两种方法，一种是使用动态创建，一种是使用静态创建。动态创建时，任务控制块和栈的内存是创建任务时动态分配的，任务删除时，内存可以释放。静态创建时，任务控制块和栈的内存需要事先定义好，是静态的内存，任务删除时，内存不能释放。目前我们以静态创建为例，configSUPPORT_STATIC_ALLOCATION在FreeRTOSConfig.h中定义，我们配置为1

* TaskFunction_t是在projdefs.h中重定义的一个数据类型，实际就是空指针，具体实现如下

> ```c
> #ifndef PROJDEFS_H
> #define PROJDEFS_H
> #include "portmacro.h"
>
> typedef void(*TaskFunction_t)(void *);
>
> #define pdFALSE         ((BaseType_t)0)
> #define pdTRUE          ((BaseType_t)1)
>
> #define pdPASS          (pdTRUE)
> #define pdFAIL          (pdFALSE)
>
> #endif/* PROJDEFS_H */
> ```

* TaskHandle_t定义如下

> ```c
> /* 任务句柄 */
> typedef void * TaskHandle_t;
> ```

* 调用prvInitialiseNewTask()函数，创建新任务，该函数在task.c中实现，具体实现如下

> ```c
> /* 创建新的任务 */
> static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,         /* 任务入口 */
>                                  const char * const pcName,         /* 任务名称，字符串形式 */
>                                  const uint32_t ulStackDepth,       /* 任务栈大小，单位为字 */
>                                  void * const pvParameters,         /* 任务形参 */
>                                  TaskHandle_t * const pxCreatedTask,/* 任务句柄 */
>                                  TCB_t *pxNewTCB)                   /* 任务控制块指针 */
> {
>     StackType_t *pxTopOfStack;
>     UBaseType_t x;
> 
>     /* 获取栈地址 */
>     pxTopOfStack = pxNewTCB->pxStack + ( ulStackDepth - ( uint32_t ) 1 );
>     /* 将栈顶指针向下做8字节对齐，在Cortex-M3（M4或M7）内核的单片机中，因为总线宽度是32位的，通常只要栈保持4字节对
>     * 齐就行，可这样为啥要8字节？难道有哪些操作是64位的？确实有，那就是浮点运算，所以要8字节对齐（但是目前我们都还
>      * 没有涉及浮点运算，只是为了后续兼容浮点运行的考虑）。如果栈顶指针是8字节对齐的，在进行向下8字节对齐的时候，指
>      * 针不会移动，如果不是8字节对齐的，在做向下8字节对齐的时候，就会空出几个字节，不会使用，比如当pxTopOfStack是33
>     * ，明显不能整除8，进行向下8字节对齐就是32，那么就会空出一个字节不使用 */
>     pxTopOfStack = ( StackType_t * ) ( ( ( uint32_t ) pxTopOfStack ) & ( ~( ( uint32_t ) 0x0007 ) ) );
>     /* 将任务的名字存储在TCB中 */
>     for( x = ( UBaseType_t ) 0; x < ( UBaseType_t ) configMAX_TASK_NAME_LEN; x++ )
>     {
>         pxNewTCB->pcTaskName[x] = pcName[x];
>         if(pcName[x] == 0x00)
>        {
>            break;
>        }
>    }
>    /* 任务名字的长度不能超过configMax_TASK_NAME_LEN，并以'0'结尾 */
>     pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';
>     /* 初始化TCB中的xStatelistItem节点，即初始化该节点所在的链表为空，表示节点还没有插入任何链表 */
>     vListInitialiseItem( &( pxNewTCB->xStateListItem ) );
>     /* 设置xStateListItem节点的拥有者，即拥有这个节点本身的TCB */
>     listSET_LIST_ITEM_OWNER( &( pxNewTCB->xStateListItem ), pxNewTCB );
>     /* 调用pxPortInitialiseStack()函数初始化任务栈，并更新栈顶指针，任务第一次运行的环境参数就
>      * 存在任务栈中。该函数在port.c（port.c第一次使用需要在freertosportableRVDSARM_CM3（ARM_CM4或者ARM_CM7）
>      * 文件夹下面新建然后添加到工程freertos/source这个组文件）中定义 */
>     pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack, pxTaskCode, pvParameters );
> 
>     /* 让任务句柄指向任务控制块 */
>     if( ( void * ) pxCreatedTask != NULL )
>     {
>        *pxCreatedTask = ( TaskHandle_t ) pxNewTCB;
>     }
> }
> ```

* 调用pxPortInitialiseStack()函数初始化任务栈，并更新栈顶指针，任务第一次运行的环境参数就存在任务栈中。该函数在port.c中定义，具体实现如下，任务栈初始化完毕之后，栈空间内部分分布图具体如下

> ```c
> StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters)
> {
>     /* 异常发生时，自动加载到CPU寄存器的内容。包括8个寄存器，分别时R0、R1、R2、R3、R12、R14、R15和xPSR的位24，且顺序不能变 */
>     pxTopOfStack--;
>     /* xPSR的bit24必须置1,即0x01000000 */
>     *pxTopOfStack = portINITAL_XPSR;
>     pxTopOfStack--;
>     /* 任务的入口地址 */
>     *pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK;
>     pxTopOfStack--;
>     /* 任务的返回地址，通常任务时不会返回的，如果返回了就跳转到preTaskExitError，该函数是一个无限循环 */
>     *pxTopOfStack = ( StackType_t ) prvTaskExitError;
>     /* R12，R3,R2 and R1默认初始化为0 */
>     pxTopOfStack -= 5;
>     *pxTopOfStack = ( StackType_t ) pvParameters;
>
>     /* 异常发生时，手动加载到CPU寄存器的内容 */
>     pxTopOfStack -= 8;
>
>     /* 返回栈顶指针，此时pxTopOfStack指向空闲栈。任务第一次运行时，就是从这个栈指针开始手动加载8个字的内容
>      * 到CPU寄存器：R4,R5,R6,R7,R8,R9,R10和R11,当推出异常时，栈中剩下的8个字的内容会自动加载到CPU寄存器：
>      * R0,R1,R2,R3,R12,R14,R15和xPSR的位24.此时PC指针就指向了任务入口地址，从而成果跳转到第一个任务 */
>     return pxTopOfStack;
> }
> ```

![pxPortInitialiseStack](https://doc.embedfire.com/rtos/freertos/zh/latest/_images/tasksw004.png)

