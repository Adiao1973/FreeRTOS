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
>```

<div align=center>
<img src="https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa008.png"
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
