#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "list.h"
#include "portmacro.h"
#include "port.h"
#include "projdefs.h"
#include "task.h"

#define configUSE_16_BIT_TICKS              0
#define configSUPPORT_STATIC_ALLOCATION     1
#define configMAX_TASK_NAME_LEN             16
#define configMAX_PRIORITIES                5

#define xPortPendSVHandler              PendSV_Handler
#define xPortSysTickHandler             SysTick_Handler
#define vPortSVCHandler                 SVC_Handler

#define configMAX_SYSCALL_INTERRUPT_PRIORITY 0xBF&0xB0

/* 不带返回值的关中断函数，不能嵌套，不能在中断里面使用。不带返回值的意
 * 思是：在往BASEPRI写入新的值的时候，不用先将BASEPRI的值保存起来，即不
 * 用管当前的中断状态是怎么样的，既然不用管当前的中断状态，也就意味着这样
 * 的函数不能在中断里面调用 
 */
#define portDISABLE_INTERRUPTS() vPortRaiseBASEPRI()

void vPortRaiseBASEPRI( void )
{
    /* configMAX_SYSCALL_INTERRUPT_PRIORITY是一个在FreeRTOSConfig.h中
     * 定义的宏，即要写入到BASEPRI寄存器的值。该宏默认定义为191，高四位有
     * 效，即等于0xb0，或者是11，即优先级大于等于11的中断都会被屏蔽，11以
     * 内的中断则不受FreeRTOS管理 
     */
    uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    __asm
    {
        /* 将configMAX_SYSCALL_INTERRUPT_PRIORITY的值写入BASEPRI寄存器，实现关中断（准确来说是关部分中断） */
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
}

/* 带返回值的关中断函数，可以嵌套，可以在中断里面使用。带返回值的意思是:在往BASEPRI写入新的值的时候，先将BASEPRI
 * 的值保存起来，在更新完BASEPRI的值的时候，将之前保存好的BASEPRI的值返回，返回的值作为形参传入开中断函数 
 */
#define portSET_INTERRUPT_MASK_FROM_ISR() ulPortRaiseBASEPRI()

void ulPortRaiseBASEPRI( void )
{
    /* configMAX_SYSCALL_INTERRUPT_PRIORITY是一个在FreeRTOSConfig.h中定义的宏，即要写入到BASEPRI寄存器的值。
     * 该宏默认定义为191，高四位有效，即等于0xb0，或者是11，即优先级大于等于11的中断都会被屏蔽，11以内的中断则
     * 不受FreeRTOS管理 
     */
    uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    __asm
    {
        /* 保存BASEPRI的值，记录当前哪些中断被关闭 */
        mrs ulReturn, basepri
        /* 更新BASEPRI的值 */
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
    /* 返回原来BASEPRI的值 */
    return ulReturn;
}

/* 不带中断保护的开中断函数，直接将BASEPRI的值设置为0，与portDISABLE_INTERRUPTS()成对使用 */
#define portENABLE_INTERRUPTS() vPortSetBASEPRI( 0 )

/* 带中断保护的开中断函数，将上一次关中断时保存的BASEPRI的值作为形参，与portSET_INTERRUPT_MASK_FROM_ISR()成对使用 */
#define  portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortSetBASEPRI(x)

/* 开中断函数，具体是将传进来的形参更新到BASEPRI寄存器。根据传入形参的不同，分为中断保护版本与非中断保护版本 */
void vPortSetBASEPRI( uint32_t ulBASEPRI )
{
    __asm
    {
        msr basepri, ulBASEPRI
    }
}


#endif /* FREERTOS_CONFIG_H */