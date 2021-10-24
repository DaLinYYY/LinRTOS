/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-23
 * @LastEditTime: 2021-08-10 13:35:32
 * @Description: 
 */
#ifndef _PORT_H
#define _PORT_H
#include <stdint.h>
#include "portcpu.h"
#include "LinOS.h"

/*  在任务切换中，主要依赖了PendSV进行切换。PendSV其中的一个很重要的作用便是用于支持RTOS的任务切换。
    1、首先将PendSV的中断优先配置为最低。这样只有在其它所有中断完成后，才会触发该中断；
        实现方法为：向NVIC_SYSPRI2写NVIC_PENDSV_PRI
    2、在需要中断切换时，设置挂起位为1，手动触发。这样，当没有其它中断发生时，将会引发PendSV中断。
        实现方法为：向NVIC_INT_CTRL写NVIC_PENDSVSET
    3、在PendSV中，执行任务切换操作。 */
#define NVIC_INT_CTRL       0xE000ED04      // 中断控制及状态寄存器
#define NVIC_PENDSVSET      0x10000000      // 触发软件中断的值
#define NVIC_SYSPRI2        0xE000ED22      // 系统优先级寄存器
#define NVIC_PENDSV_PRI     0x000000FF      // 配置优先级

#define MEM32(addr)         *(volatile unsigned long *)(addr)
#define MEM8(addr)          *(volatile unsigned char *)(addr)



/* 临界区相关函数 */
uint32_t uTaskEnterCritical(void);
void vTaskExitCritical(uint32_t status);

/* 硬件调度 */
void taskScheduler(void);
void taskFristRun(void);

void vTaskSetTickPeriod(uint32_t time);

/**
 * @Author: YangSL
 * @Description: 
 * @param {tTask *} task
 * @param {void} *
 * @param {void} *param
 * @param {uint32_t *} stack
 * @param {uint32_t} size
 */
void hwStackInit(tTask * task, void (*entry)(void *), void *param, uint32_t * stack, uint32_t stackSize);

#endif /* _PORT_H */
