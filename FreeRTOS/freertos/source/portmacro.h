#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "FreeRTOSConfig.h"
/* 包含标准库头文件 */
#include "stdint.h"
#include "stddef.h"

/* 数据类型重定义 */
#define portCHAR            char
#define portFLOAT           float
#define portDOUBLE          double
#define portLONG            long
#define portSHORT           short
#define portSTACK_TYPE      uint32_t
#define portBASE_TYPE       long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

/* 定义任务栈 */
#define TASK1_STACK_SIZE    128
StackType_t Task1Stack[TASK1_STACK_SIZE];

#define TASK2_STACK_SIZE    128
StackType_t Task2Stack[TASK2_STACK_SIZE];

/* 定义任务控制块 */
TCB_t Task1TCB;
TCB_t Task2TCB;

/* 定义任务句柄 */
TaskHandle_t Task1_Handle;
TaskHandle_t Task2_Handle;


#if( configUSE_16_BIT_TICKS == 1)
typedef uint16_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffffffffUL
#endif

/* 中断控制状态寄存器：0xe000ed04
 * Bit 28 PENDSVSET:PendSV 悬起位 
 */
#define portNVIC_INT_CTRL_REG       (*(( volatile uint32_t * )0xe000ed04))
#define portNVIC_PENDSVSET_BIT      (1UL << 28UL)

#define portSY_FULL_READ_WRITE      (15)

/* portYIELD的实现很简单，实际就是将PendSV的悬起位置1，当没有其他中断运行的时候响应PendSV中断，
 * 去执行我们写好的PendSV中断服务函数，在里面实现任务切换 */
#define portYIELD()\
{\
    /* 触发PendSV，产生上下文切换 */\
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;\
    __dsb(portSY_FULL_READ_WRITE);\
    __isb(portSY_FULL_READ_WRITE);\
}

/* 进入临界段 */
/* 不带中断保护版本，不能嵌套 */
#define portENTER_CRITICAL()        vPortEnterCritical()
#define portDISABLE_INTERRUPTS()    vPortRaiseBASEPRI()

static portFORCE_INLINE void vPortRaiseBASEPRI( void )
{
    uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    __asm
    {
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
}

/* 带中断保护版本，可以嵌套 */
#define portSET_INTERRUPT_MASK_FORM_ISR()   ulPortRaiseBASEPRI()

static portFORCE_INLINK uint32_t ulPortRaiseBASEPRI( void )
{
    uint32_t ulReturn,ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

    __asm
    {
        mrs ulReturn, basepri
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
    return ulReturn;
}

/* 退出临界段 */
 


#endif/* PORTMACRO_H */