/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-23 
 * @LastEditTime: 2021-08-10 16:15:55
 * @Description: 
 */
#include "port.h"


/*---------------------------------------------------------------------------------------------------------------------
 * 硬件调度相关操作
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 调用PendSV异常发起任务切换请求
 */
void taskScheduler(void)
{
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;    // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV
}

/**
 * @Author: YangSL
 * @Description: 系统运行的第一个任务
 */
void taskFristRun(void)
{
    /* 设置 PSP = MSP, 二者都指向同一个堆栈 */
    __set_PSP(__get_MSP());

    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;   // 向NVIC_SYSPRI2写NVIC_PENDSV_PRI，设置其为最低优先级
    
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;    // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV
}

/*---------------------------------------------------------------------------------------------------------------------
 * 临界区相关操作
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 进入临界区
 */
uint32_t uTaskEnterCritical(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

/**
 * @Author: YangSL
 * @Description: 退出临界区
 * @param {uint32_t} status
 */
void vTaskExitCritical(uint32_t status)
{
    __set_PRIMASK(status);
}

/*---------------------------------------------------------------------------------------------------------------------
 * 系统硬件设置
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 设置触发SysTick异常的的周期
 * @param {uint32_t} timetime
 */
void vTaskSetTickPeriod(uint32_t time)
{
    /* System Core Clock update function */
    SystemCoreClockUpdate();
    
    SysTick->LOAD  = time * SystemCoreClock / 1000 - 1; 
    NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->VAL   = 0;                           
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk; 
}


/* SysTick中断服务函数 */
void SysTick_Handler () 
{
    vTaskSystemTickHandler();
}


uint32_t loadStackAddress(uint32_t *stackAddress )
{
    /* 第一次切换时currentTask == 0，不需要保存 */
    if(currentTask != (tTask*)0)
    {
        currentTask->stack = (uint32_t *)stackAddress;
    }
    currentTask = nextTask;
    /* 返回下一个任务的堆栈地址 */
    return (uint32_t) currentTask->stack;
}

/*---------------------------------------------------------------------------------------------------------------------
 * PendSV_Handler  :   放到最后不然会使得struct变量无法加载 
 *--------------------------------------------------------------------------------------------------------------------*/
#if defined(__TARGET_CPU_CORTEX_M0)

void hwStackInit(tTask * task, void (*entry)(void *), void *param, uint32_t * stack, uint32_t size)
{
    /* 定义栈顶 */
    uint32_t * stackTop;

    task->stackBase = stack;
    task->stackSize = stackSize;
    /* 初始化堆桟中的值 */

    memset(stack, 0, stackSize);

    stackTop = task->stackBase + task->stackSize / sizeof(tTaskStack);

    *(--stackTop) = (unsigned long)(1<<24);                        // XPSR, 设置了Thumb模式
    *(--stackTop) = (unsigned long)entry;                          // 程序的入口地址
    *(--stackTop) = (unsigned long)0x14;                           // R14(LR)
    *(--stackTop) = (unsigned long)0x12;                           // R12, 未用
    *(--stackTop) = (unsigned long)0x3;                            // R3, 未用
    *(--stackTop) = (unsigned long)0x2;                            // R2, 未用
    *(--stackTop) = (unsigned long)0x1;                            // R1, 未用
    *(--stackTop) = (unsigned long)param;                          // R0 = param, 传给任务的入口函数

    *(--stackTop) = (unsigned long)0x7;                            // R7, 未用
    *(--stackTop) = (unsigned long)0x6;                            // R6, 未用
    *(--stackTop) = (unsigned long)0x5;                            // R5, 未用
    *(--stackTop) = (unsigned long)0x4;                            // R4, 未用
    
    *(--stackTop) = (unsigned long)0x11;                           // R11, 未用
    *(--stackTop) = (unsigned long)0x10;                           // R10, 未用
    *(--stackTop) = (unsigned long)0x9;                            // R9, 未用
    *(--stackTop) = (unsigned long)0x8;                            // R8, 未用


    task->stack             = stackTop;                            // 保存最终的值    
}

#if 1   /* PendSV纯汇编代码 */
__asm void PendSV_Handler ()
{   
    IMPORT  loadStackAddress      
    
    MRS     R0, PSP                     // 获取当前任务的堆栈指针
    
    MOVS    R1, #32
    SUBS    R0, R1                      // 预先计算, R0此时得出的最后的堆栈地址,方便后面用stmia保存
                                        //     保存的地址是当前任务的PSP堆栈中,便于下次恢复
    STMIA   R1!, {R4-R7}                //     将除异常自动保存的寄存器这外的其它寄存器自动保存起来{R4, R11}
    MOV     R4, R8                      // 在cortex-m0不STMDB不支持访问R8~R11寄存器,所以下面通过R4~R7间接写入
    MOV     R5, R9
    MOV     R6, R10
    MOV     R7, R11
    STMIA   R1!, {R4-R7}

    BL      loadStackAddress            // 调用函数：参数通过R0传递，返回值也通过R0传递 
    
    LDMIA   R0!, {R4-R7}                // cortex-m0不支持LDMIA访问R8-R11,所以通过R4-R7间接取出
    MOV     R8, R4                      
    MOV     R9, R5
    MOV     R10, R6
    MOV     R11, R7
    LDMIA   R0!, {R4-R7}                // 取出R4-R7

    MSR     PSP, R0                     // 最后，恢复真正的堆栈指针到PSP

    MOVS    R0, #2                      // 生成0xFFFFFFFD
    MVNS    R0, R0
    BX      R0                          // 最后返回，此时任务就会从堆栈中取出LR值，恢复到上次运行的位置
   
}  
#else /* PendSV汇编 + C代码 */

__asm void PendSV_Handler(void){
    IMPORT  switchTaskStack

    MRS     R0, PSP
    STMDB   R0!, {R4-R11}             // 将R4~R11保存到当前任务栈，也就是PSP指向的堆栈
    BL      switchTaskStack           // 调用函数：参数通过R0传递，返回值也通过R0传递 
    LDMIA   R0!, {R4-R11}
    MSR     PSP, R0

    MOV     LR, #0XFFFFFFFD
    BX      LR
}

uint32_t switchTaskStack(uint32_t stackAddr)
{
    if(currentTask != (tTask *)0){
        currentTask->stack =(uint32_t *)stackAddr;
    }
    currentTask = nextTask;
    return (uint32_t)currentTask->stack;
}
#endif 

#elif defined(__TARGET_CPU_CORTEX_M3) || defined(__TARGET_CPU_CORTEX_M4)

void hwStackInit(tTask * task, void (*entry)(void *), void *param, uint32_t * stack, uint32_t stackSize)
{
    /* 定义栈顶 */
    uint32_t * stackTop;

    task->stackBase = stack;
    task->stackSize = stackSize;
    /* 初始化堆桟中的值 */

    memset(stack, 0, stackSize);

    stackTop = task->stackBase + task->stackSize / sizeof(tTaskStack);

    *(--stackTop) = (unsigned long)(1<<24);                        // XPSR, 设置了Thumb模式
    *(--stackTop) = (unsigned long)entry;                          // 程序的入口地址
    *(--stackTop) = (unsigned long)0x14;                           // R14(LR)
    *(--stackTop) = (unsigned long)0x12;                           // R12, 未用
    *(--stackTop) = (unsigned long)0x3;                            // R3, 未用
    *(--stackTop) = (unsigned long)0x2;                            // R2, 未用
    *(--stackTop) = (unsigned long)0x1;                            // R1, 未用
    *(--stackTop) = (unsigned long)param;                          // R0 = param, 传给任务的入口函数

    *(--stackTop) = (unsigned long)0x11;                           // R11, 未用
    *(--stackTop) = (unsigned long)0x10;                           // R10, 未用
    *(--stackTop) = (unsigned long)0x9;                            // R9, 未用
    *(--stackTop) = (unsigned long)0x8;                            // R8, 未用

    *(--stackTop) = (unsigned long)0x7;                            // R7, 未用
    *(--stackTop) = (unsigned long)0x6;                            // R6, 未用
    *(--stackTop) = (unsigned long)0x5;                            // R5, 未用
    *(--stackTop) = (unsigned long)0x4;                            // R4, 未用

    task->stack             = stackTop;                            // 保存最终的值    
}

__asm void PendSV_Handler ()
{   
    IMPORT  loadStackAddress      
    
    /* 保存当前上下文 */
    MRS     R0, PSP                   // 获取当前任务的堆栈指针

    MOVS    R1, #32
    SUBS    R0, R1                    // 预先计算, R0此时得出的最后的堆栈地址,方便后面用stmia保存
                                      //     保存的地址是当前任务的PSP堆栈中,便于下次恢复
    STMIA   R1!, {R4-R7}              //     将除异常自动保存的寄存器这外的其它寄存器自动保存起来{R4, R11}
    MOV     R4, R8                    // 在cortex-m0不STMDB不支持访问R8~R11寄存器,所以下面通过R4~R7间接写入
    MOV     R5, R9
    MOV     R6, R10
    MOV     R7, R11
    STMIA   R1!, {R4-R7}

    BL      loadStackAddress        // 调用函数：参数通过R0传递，返回值也通过R0传递 
    
    LDMIA   R0!, {R4-R7}                // cortex-m0不支持LDMIA访问R8-R11,所以通过R4-R7间接取出
    MOV     R8, R4                      
    MOV     R9, R5
    MOV     R10, R6
    MOV     R11, R7
    LDMIA   R0!, {R4-R7}                // 取出R4-R7

    MSR     PSP, R0                     // 最后，恢复真正的堆栈指针到PSP

    MOVS    R0, #2                      // 生成0xFFFFFFFD
    MVNS    R0, R0
    BX      R0                          // 最后返回，此时任务就会从堆栈中取出LR值，恢复到上次运行的位置
   
}

#endif
