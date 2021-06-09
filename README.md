# FreeRTOS

Self learning FreeRTOS

## 1. FreeRTOS中链表的实现 

FreeRTOSS中与链表相关的操作均在list.h和list.c这两个文件中实现，list.h第一次使用需要在include文件夹下面新建然后添加到工程
freertos/source这个组文件，list.c第一次使用需要在freertos文件夹下面新建然后添加到工程freetos/source这个组文件。

### 1.1 实现链表节点

#### 1.1.1 定义链表节点数据结构

链表节点的数据结构在list.h中定义，具体实现和节点示意图如下

```c
struct xLIST_ITEM
{
    TickType_t xItemValue; /* 辅助值，用于帮助节点做顺序排序*/ (1)
    struct xLIST_ITEM * pxNext; /* 指向链表下一个节点 */ (2)
    struct xLIST_ITEM * pxPrevious; /* 指向链表前一个节点 */ (3)
    void * pvOwner; /* 指向拥有该节点的内核对象，通常是TCB */ (4)
    void * pvContainner; /* 指向该节点所在的链表 */ (5)
};
typedefstruct xLIST_ITEM ListItem_t
```

<div align=center>
<img src="https://doc.embedfire.com/rtos/freertos/zh/latest/_images/listsa008.png"
</div>

* (1):一个辅助值，用于帮助节点做顺序排列。该辅助值的数据类型为TickType_t，在FreeRTOS中，凡是涉及数据类型的地方，FreeRTOS都会将标准的C数据类型用typedef 重新取一个类型名。这些经过重定义的数据类型放在 portmacro.h（portmacro.h第一次使用需要在include文件夹下面新建然后添加到工程freertos/source这个组文件）这个头文件,具体代码如下

```c
#ifndef PORTMACRO_H
#define PORTMACRO_H

#include"stdint.h"
#include"stddef.h"

/* 数据类型重定义 */
#define portCHAR            char
#define portFLOAT           float
#define portDOUBLE          double
#define portLONG            long
#define portSHORT           short
#define portSTACK_TYPE      uint32_t
#define portBASE_TYPE       long

typedef portSTACK_TYPE StackType_t;
typedeflong BaseType_t;
typedefunsigned long UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )(1)
typedefuint16_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffff
#else
typedefuint32_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif

#endif/* PORTMACRO_H */
```
