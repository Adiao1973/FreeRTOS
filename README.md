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