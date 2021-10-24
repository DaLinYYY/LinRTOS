/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-08-09 21:46:45
 * @LastEditTime: 2021-08-09 23:01:47
 * @Description: 
 */

#ifndef _LINOS_CONFIG_H
#define _LINOS_CONFIG_H

/* LinOS优先级数 */
#define LinOS_PRI_COUNT                         32

/* 每个任务最大运行的时间片计数 */
#define LinOS_SLICE_MAX                         10

/* 堆桟大小 */
#define LinOS_STACK_SIZE		                1024
#define LinOS_IDLE_STACK_SIZE		            1024


/* 时钟节拍的周期，以ms为单位 */
#define LinOS_SYSTICK_MS                1

#define LinOS_TIMER_PRIO                1

/* 是否使用定时器 */
#define LinOS_TIMER_SWITCH              0

/* 是否进行CPU使用率检测 */
#define LinOS_ENABLE_CPU_CHECK          1

#endif /* _LINOS_CONFIG_H */
