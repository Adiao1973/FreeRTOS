#include "port.h"

static void prvTaskExitError( void )
{
    /* 函数停止在这里 */
    for(;;);
}

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters)
{
    /* 异常发生时，自动加载到CPU寄存器的内容。包括8个寄存器，分别时R0、R1、R2、R3、R12、R14、R15和xPSR的位24，且顺序不能变 */
    pxTopOfStack--;
    /* xPSR的bit24必须置1,即0x01000000 */
    *pxTopOfStack = portINITAL_XPSR;
    pxTopOfStack--;
    /* 任务的入口地址 */
    *pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK;
    pxTopOfStack--;
    /* 任务的返回地址，通常任务时不会返回的，如果返回了就跳转到preTaskExitError，该函数是一个无限循环 */
    *pxTopOfStack = ( StackType_t ) prvTaskExitError;
    /* R12，R3,R2 and R1默认初始化为0 */
    pxTopOfStack -= 5;
    *pxTopOfStack = ( StackType_t ) pvParameters;

    /* 异常发生时，手动加载到CPU寄存器的内容 */
    pxTopOfStack -= 8;

    /* 返回栈顶指针，此时pxTopOfStack指向空闲栈。任务第一次运行时，就是从这个栈指针开始手动加载8个字的内容
     * 到CPU寄存器：R4,R5,R6,R7,R8,R9,R10和R11,当推出异常时，栈中剩下的8个字的内容会自动加载到CPU寄存器：
     * R0,R1,R2,R3,R12,R14,R15和xPSR的位24.此时PC指针就指向了任务入口地址，从而成果跳转到第一个任务 */
    return pxTopOfStack;
}

/* 
 * 参考资料《STM32F10xxx Cortex-M3 programming manual》4.4.3，百度搜索“PM0056”即可找到这个文档
 * 在Cortex-M中，内核外设SCB中SHPR3寄存器用于设置SysTick和PendSV的异常优先级
 * System handler priority register 3 (SCB_SHPR3) SCB_SHPR3：0xE000 ED20
 * Bits 31:24 PRI_15[7:0]: Priority of system handler 15, SysTick exception
 * Bits 23:16 PRI_14[7:0]: Priority of system handler 14, PendSV
 */
#define portNVIC_SYSPRI2_REG                ( ( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_PENDSV_PRI(((uint32_t) configKERNEL_INTERRUPT_PRIORITY ) << 16UL)
#define portNVIC_SYSTICK_PRI(((uint32_t) configKERNEL_INTERRUPT_PRIORITY ) << 24UL)

BaseType_t xPortStartScheduler( void )
{
    /* 配置PendSV 和 SysTick 的中断优先级为最低 */
    portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;

    /* 启动第一个任务，不再返回 */
    prvStartFirstTask();

    /* 不应该运行到这里 */
    return 0;
}

/*
 * 参考资料《STM32F10xxx Cortex-M3 programming manual》4.4.3,百度搜索“PM0056”即可找到这个文档
 * 在Cortex-M中，内核外设SCB的地址范围为：0xE00ED00-0xE000ED3F
 * 0xE000ED008为SCB外设中SCB外设中SCB_VTOR这个寄存器的地址，里面存放的室向量表的起始地址，即MSP的地址
 */

__asm void prvStartFirstTask( void )
{
    /* 当前栈需按照8字节对齐，如果都是32位的操作则4个字节对齐即可。在Cortex-M中浮点运算是8字节的 */    
    PRESERVE8

    /* Cortext-M3硬件中，0xE000ED08地址为SCB_VTOR(向量表偏移量)寄存器，储存向量表起始地址,即MSP的地址。
     *向量表通常是从内部FLASH的起始地址开始存放，那么可知memory:0x00000000处存放的就是MSP的值 */
    /* 将0xE000ED08这个立即数加载到寄存器R0 */
    ldr r0, =0xE000ED08
    /* 将0xE000ED08这个地址指向的内容加载到寄存器R0，此时R0等于SCB_VTOR寄存器的值，等于0x00000000，即memory的起始地址 */
    ldr r0, [r0]
    /* 取出向量表中的第一项，向量表第一项储存主堆栈指针MSP的初始值。将0x00000000这个地址指向的内容加载到R0，此时R0等于0x200008DB */
    ldr r0, [r0]

    /* 将堆栈地址存入主堆栈指针 将R0的值储存到MSP，此时MSP等于0x200008DB，这是主栈的栈顶指针。起始这一步有点多余，因为当系统启动的时候，
     * 执行完Reset_Handler的时候，向量表已经初始化完毕，MSP的值已经更新为向量表的起始值，即指向主栈的栈顶指针*/
    msr msp, r0

    /* 使能全局中断 */
    /* 使用CPS指令把全局中断打开。为了快速的开关中断，Cotex-M内核专门设置了一条CPS指令，有4种用法 */
    cpsie i
    cpsie f
    dsb 
    isb

    /* 触发SVC中断开启第一个任务 */
    /* 产生系统调用，服务号0表示SVC中断，接下来将会执行SVC中断服务函数 */
    svc 0
    nop
    nop
}

__asm void vPortSVCHandler( void )
{
    /* 声明外部变量pxCurrentTCB,pxCurrentTCB是一个在task.c中定义的全局指针，
     * 用于指向当前正在运行或者即将要运行的任务的任务控制块 */
    extern pxCurrentTCB;

    PRESERVE8
    /* 加载pxCurrentTCB的地址到r3 */
    ldr r3,=pxCurrentTCB
    /* 加载pxCurrentTCB到r1 */
    ldr r1,[r3]
    /* 加载pxCurrentTCB指向的任务控制块到r0,任务控制块的第一个成员就是栈顶指针，所以此时r0等于栈顶指针。 */
    ldr r0,[r1]
    /* 以r0为基地址，将栈中向上增长的8个字的内容加载到CPU寄存器r4~r11，同时r0也会跟着自增 */
    ldmia r0!,{r4-r11}
    /* 将新的栈顶指针r0更新到psp，任务执行的时候使用的栈指针式是psp */
    msr psp,r0

    isb
    /* 将寄存器r0清0 */
    mov r0,#0
    /* 设置basepri寄存器的值为0，即打开所有中断。basepri是一个中断屏蔽寄存器，大于等于此寄存器值的中断都将被屏蔽 */
    msr basepri,r0
    /* 当从SVC中断服务退出前，通过向r14寄存器最后4位按位或上0x0D，使得硬件在退出时使用进程栈指针PSP完成出栈操作并
     * 返回后进入任务模式、返回Thumb状态。在SVC中断服务里面，使用的时MSP栈指针，是处在ARM状态 */
    orr r14,#0xd

    /* 异常返回，这个时候出栈使用的是PSP指针，自动将栈中的剩下内容加载到CPU寄存器：
     * xPSR,PC(任务入口地址)，R14,R12,R3,R2,R1,R0(任务的形参)同时PSP的值也将更新，即指向任务栈的栈顶。 */
    bx r14
}

__asm void xPortPendSVHandler( void )
{
    /* 声明外部变量pxCurrentTCB,pxCurrentTCB是一个在task.c中定义的全局指针，用于指向当前正在运行或者即将要进行的任务的任务控制块 */
    extern pxCurrentTCB;
    /* 声明外部函数vTaskSwitchContext */
    extern vTaskSwitchContext;

    /* 当前栈需按照8字节对齐，如果都是32位的操作则4个字节对齐即可。在Cortex-M中浮点运算是8字节的 */
    PRESERVE8

    /* 将PSP的值储存到r0。当进入PendSVC Handler时，上一个任务运行的环境即：
     * xPSR,PC(任务入口地址),R14,R12,R3,R2,R1,R0(任务的形参)这些CPU寄存器
     * 的值回自动储存到任务的栈中，剩下的r4~r11需要手动保存，同时PSP会自动更
     * 新(在更新之前PSP指向任务栈的栈顶) 
     */
    mrs r0, psp
    isb

    /* 加载pxCurrentTCB的地址到r3 */
    ldr r3,=pxCurrentTCB
    /* 加载r3指向的内容到r2，即r2等于pxCurrentTCB */
    ldr r2, [r3]

    /* 以r0作为基址(指针先递减，再操作，STMDB的DB表示DecreaseBefor),将CPU寄存器r4~r11的值储存到任务栈，同时更新r0的值 */
    stmdb r0!, {r4-r11}
    /* 将r0的值储存到r2指向的内容，r2等于pxCurrentTCB。具体为将r0的值储存到上一个任务的栈顶指针pxTopOfStack，至此，上下文切换中的上文保存就完成了 */
    str r0, [r2]

    /* 将R3和R14临时压入栈(在整个系统中，中断使用的是主栈，栈指针使用的是MSP)，因为接下来要调用函数vTaskSwitchContext，调用函数时，返回地址自动保
     * 存到R14中，所以一旦调用发生，R14的值会被覆盖(PendSV中断服务函数执行完毕后，返回的时候需要根据R14的值来决定返回处理器模式还是任务模式，出栈时
     * 使用的是PSP还是MSP)，因此需要入栈保护。R3保存的是当前正在运行的任务(准确来说是上文，因为接下来即将要切换到新的任务)的TCB指针(pxCurrentTCB)地
     * 址，函数调用后pxCurrentTCB的值会被更新，后面我们还需要通过R3来操作pxCurrentTCB，但是运行函数vTaskSwitchContext时不确定会不会使用R3寄存器作
     * 为中间变量，所以为了保险起见，R3也入栈保护起来
     */
    stmdb sp!, {r3,r14}

    /* 将configMAX_SYSCALL_INTERRUPT_PRIORITY的值储存到r0，该宏在FreeRTOSConfig.h中定义，用来配置中断屏蔽寄存器BASEPR的值，高四位有效。目前配置为191，
     * 因为是高四位有效，所以实际值等于11，即优先级高于或者等于11的中断都将被屏蔽。在关中断方面，FreeRTOS与其他的RTOS关中断不同，而是操作BASEPRI寄存器来预
     * 留一部分中断，并不像μC、OS或者RT-Thread那样直接操作PRIMASK把所有中断都关闭掉（除了硬FAULT） 
     */
    mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
    /* 关中断，进入临界段，因为接下来要更新全局指针pxCurrentTCB的值 */
    msr basepri, r0
    dsb
    isb
    /* 调用函数vTaskSwitchContext。该函数在task.c中定义，作用只有一个，选择优先级最高的任务，然后更新pxCurrentTCB。目前我们还不支持优先级，则手动切换，不是任务1就是任务2 */
    bl vTaskSwitchContext
    /* 退出临界段，开中断，直接往BASEPRI写0 */
    mov r0, #0
    msr basepri, r0
    /* 从主栈中恢复寄存器r3和r14的值，此时的sp使用的是MSP */
    ldmia sp!,{r3,r14}

    /* 加载r3指向的内容到r1。r3存放的是pxCurrentTCB的地址，即让r1等于pxCurrentTCB。pxCurrentTCB在上面的vTaskSwitchContext函数中被更新，指向了下一个将要运行的任务的TCB */
    ldr r1, [r3]
    /* 加载r1的指向的内容到r0，即下一个要运行的任务的栈顶指针 */
    ldr r0, [r1]
    /* 将r0作为基地址（先取值，再递增指针，LDMIA的IA表示IncreaseAfter），将下一个要运行的任务栈的内容加载到CPU寄存器r4~r11 */
    ldmia r0!, {r4-r11}
    /* 更新psp的值，等下异常退出时，会以psp作为基地址，将任务栈中剩下的内容自动加载到CPU寄存器 */
    msr psp, r0
    isb
    /* 异常发生时，R14中保存异常返回标志，包括返回后进入任务模式还是处理器模式、使用PSP栈指针还是MSP栈指针。此时的r14等于0xfffffffd，表示异常返回后进入任务模式，SP以PSP作为
     * 栈指针出栈，出栈完毕后PSP指向任务栈的栈顶。当调用bx r14指令后，系统以PSP作为SP指针出栈，把接下来要运行的新任务的任务栈中剩下的内容加载到CPU寄存器：R0（任务形参）、R1、
     * R3、R12、R14（LR）、R15（PC）和xPSR，从而切换到新的任务 */
    bx r14
    nop
}

void vPortEnterCritical( void )
{
    portDISABLE_INTERRUPTS();
    /* uxCriticalNesting是在port.c中定义的静态变量，表示临界段嵌套计数器，默认初始化为0xaaaaaaaa,在调度器启动时会被重新初始化为0：
     * vTaskStartScheduler()->xPortStartScheduler()->uxCriticalNesting = 0 
     */
    uxCriticalNesting++;

    /* 如果uxCriticalNesting等于1，即一层嵌套，要确保当前没有中断活跃，即内核外设SCB中的中断和控制寄存器SCB_ICSR的低8位要等于0。
     * 有关SCB_ICSR的具体描述可参考“STM32F10XXX Cortex-M3 programmingmanual-4.4.2” 
     */
    if( uxCriticalNesting == 1 )
    {
        configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
    }
}