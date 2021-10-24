/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-08-07 11:32:44
 * @LastEditTime: 2021-08-08 17:08:38
 * @Description: 
 */

#ifndef _TIMER_H
#define _TIMER_H

#include "Event.h"

extern long long timerGlobalTick;

#define LinOS_TIMERTASK_STACK_SIZE      512
#define LinOS_TIMERTASK_PRIO            1
/*---------------------------------------------------------------------------------------------------------------------
 * TIMER DEFINITIONS
 *--------------------------------------------------------------------------------------------------------------------*/
typedef enum _eTimerState
{
    eTimerCreated,          // 定时器已经创建
    eTimerStarted,          // 定时器已经启动
    eTimerRunning,          // 定时器正在运行
    eTimerStopped,          // 定时器已经停止
    eTimerDeleted           // 定时器已经销毁
}eTimerState;

typedef struct _tTimer
{
    /* 链表节点 */
    tNode linkNode;
    
    /* 定时时间 */
    uint32_t timerTicks;
    /* 该定时器任务当前时间 */
    uint32_t delayTicks;
    /* 定时器类型 */
    uint32_t flag;

    /* 定时回调函数 */
    void (*timerFunc) (void * arg);
    /* 传递给回调函数的参数 */
    void *arg;

    /* 定时器配置参数 */
    uint32_t config;
    /* 定时器状态 */
    eTimerState state;
}tTimer;


#define TIMER_MODE_MASK   0x1
#define TIMER_HARD_MODE   (1 << 0)              /* 硬件定时器 */
#define TIMER_SOFT_MODE   (0 << 0)              /* 软件定时器 */

#define TIMER_FLAG_MASK         0x2
#define TIEMR_FLAG_ONE_SHOT     (0 << 1)        /* 一次性定时器 */
#define TIEMR_FLAG_PERIODIC     (1 << 1)        /* 周期性定时器 */


/*---------------------------------------------------------------------------------------------------------------------
 * TIMER API
 *--------------------------------------------------------------------------------------------------------------------*/

/**
 * @Author: YangSL
 * @Description: 初始化定时器
 * @param {tTimer *} timer          等待初始化的定时器
 * @param {uint32_t} timerTicks     定时器超时Ticks
 * @param {uint32_t} flag           设置定时器是一次性还是周期定时器
 * @param {void *} timerFunc        定时器回调函数
 * @param {void *} arg              传递给定时器回调函数的参数
 * @param {uint32_t} config         定时器的初始配置
 */
void vTimerInit(tTimer * timer, uint32_t timerTicks, uint32_t flag, void (*timerFunc)(void * arg), void * arg, uint32_t config);


/**
 * @Author: YangSL
 * @Description: 检查定时器模块的时间计数
 */
void vTimerModulCheckTick(void);

/**
 * @Author: YangSL
 * @Description: 启动定时器任务
 * @param {tTimer} *timer
 */
void vTimerStart(tTimer *timer);

/**
 * @Author: YangSL
 * @Description: 停止定时器任务
 * @param {tTimer *} timer
 */
void vTimerStop(tTimer * timer);

/**
 * @Author: YangSL
 * @Description: 创建定时器模块
 */
void vTimerModulInit(void);
/**
 * @Author: YangSL
 * @Description: 创建定时器任务
 */
void vTimerTickInit(void);

#endif /* _TIMER_H */
