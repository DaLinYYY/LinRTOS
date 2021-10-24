/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-08-07 11:32:51
 * @LastEditTime: 2021-08-09 21:44:29
 * @Description: 
 */
#include "LinOS.h"

/* "硬"定时器列表 */
static tList timerHardList;
/* "软"定时器列表 */
static tList timerSoftList;

/* 用于访问软定时器列表的信号量 */
static tSem timerProtectSem;
/* 用于软定时器任务与中断同步的计数信号量 */
static tSem timerTickSem;


/* 定义定时器任务和任务的栈空间大小 */
static tTask timerTask;
static tTaskStack timerTaskStack[LinOS_TIMERTASK_STACK_SIZE];

/*---------------------------------------------------------------------------------------------------------------------
 * TIMER FUNC
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
void vTimerInit(tTimer * timer, uint32_t timerTicks, uint32_t flag, void (*timerFunc)(void * arg), void * arg, uint32_t config)
{
    /* 初始化定器列表 */
    vNodeInit(&timer->linkNode);

    /* 设置定时器类型及定时时间 */
    timer->timerTicks       = timerTicks;
    timer->flag             = flag;
    /* 当前定时器递减值 */
    timer->delayTicks       = timerTicks;

    /* 设置定时器的超时回调函数 */
    timer->timerFunc        = timerFunc;
    timer->arg              = arg;

    /* 配置模式 */
    timer->config           = config;

    /* 设定时器标志为：创建完成 */
    timer->state            = eTimerCreated;

}


/**
 * @Author: YangSL
 * @Description: 
 * @param {tList *} timerList
 */
static void vTimerCallFuncList(tList * timerList)
{
    tNode * node;

    /* 遍历所有的定时任务 */
    for(node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
    {
        tTimer * timer = nodeParent(node, tTimer, linkNode);

        /* 如果延时已到，则调用定时器处理函数 */
        if(timer->delayTicks == 0 || --timer->delayTicks == 0)
        {
            timer->state = eTimerRunning;

            /* 调用定时器回调函数 */
            timer->timerFunc(timer->arg);

            timer->state = eTimerStarted;

            if((timer->flag & TIMER_FLAG_MASK) == TIEMR_FLAG_PERIODIC)
            {
                /* 周期定时，重新定时计数 */
                timer->delayTicks = timer->timerTicks;
            }
            else
            {
                /* 是一次性计数器，中止定时器 */
                vListRemoveNode(timerList, &timer->linkNode);
                timer->state = eTimerStopped;
            }
        }

    }
}

/**
 * @Author: YangSL
 * @Description: 检查定时器模块的时间计数
 */
void vTimerModulCheckTick(void)
{
    uint32_t status = uTaskEnterCritical();

    /* 处理硬定时器列表 */
    vTimerCallFuncList(&timerHardList);

    vTaskExitCritical(status);

    /* 释放信号量通知软件定时器节拍发生变化 */
    vSemRelease(&timerTickSem);
}

/**
 * @Author: YangSL
 * @Description: 启动定时器任务
 * @param {tTimer} *timer
 */
void vTimerStart(tTimer *timer)
{
    switch (timer->state)
    {
    case eTimerCreated:
    case eTimerStopped:
        timer->delayTicks = timer->timerTicks;
        timer->state = eTimerStarted;

        if((timer->config & TIMER_MODE_MASK) == TIMER_HARD_MODE)
        {
            /* 硬件定时器模式 */
            uint32_t status = uTaskEnterCritical();

            /* 加入硬定时器列表 */
            vListInsertLast(&timerHardList, &timer->linkNode);

            vTaskExitCritical(status);
        }
        else
        {
            uSemTake(&timerProtectSem, 0);
            vListInsertLast(&timerSoftList, &timer->linkNode);
            vSemRelease(&timerProtectSem);
        }
        break;
    
    default:
        break;
    }
}


/**
 * @Author: YangSL
 * @Description: 停止定时器任务
 * @param {tTimer *} timer
 */
void vTimerStop(tTimer * timer)
{
    switch (timer->state)
    {
    case eTimerStarted:
    case eTimerRunning:
        
        /* 判断定时器模式 */
        if((timer->config & TIMER_MODE_MASK) == TIMER_HARD_MODE)
        {
            /* 硬件定时器模式 */
            uint32_t status = uTaskEnterCritical();

            vListRemoveNode(&timerHardList, &timer->linkNode);

            vTaskExitCritical(status);
        }
        else
        {
            /* 软件定时器模式 */
            uSemTake(&timerProtectSem, 0);
            vListRemoveNode(&timerSoftList, &timer->linkNode);
            vSemRelease(&timerProtectSem);
        }
        break;
    
    default:
        break;
    }
}


/**
 * @Author: YangSL
 * @Description: 软件定时器任务
 * @param {void *} param
 */
static void timerSoftTask(void * param)
{
    for(;;)
    {
        /* 无限制等待等待系统节拍发送的中断事件信号 */
        uSemTake(&timerTickSem, 0);

        /* 无限制等待软定时器列表的访问权限 */
        uSemTake(&timerProtectSem, 0);

        /* 处理软件定时器列表 */
        vTimerCallFuncList(&timerSoftList);

        /* 释放定时器访问权限 */
        vSemRelease(&timerProtectSem);
    }
}

/**
 * @Author: YangSL
 * @Description: 创建定时器模块
 */
void vTimerModulInit(void)
{
    /* 初始化全局定时器 */


    /* 初始化硬件计数列表 */
    vListInit(&timerHardList);
    /* 初始化软件定时器列表 */
    vListInit(&timerSoftList);
    /*  */
    vSemInit(&timerProtectSem, 1, 1);
    /* 初始化软定时器任务与中断同步的计数信号量 */
    vSemInit(&timerTickSem, 0, 0);

}

/**
 * @Author: YangSL
 * @Description: 创建定时器任务
 */
void vTimerTickInit(void)
{
#if LinOS_TIMER_PRIO >= LinOS_PRI_COUNT -1
    #error "The proprity of timer task must be greater then (LinOS_PRI_COUNT - 1)"
#endif
    /* 初始化定时器任务 */
    vTaskInit(&timerTask, timerSoftTask, (void *)0, LinOS_TIMERTASK_PRIO, timerTaskStack, LinOS_TIMERTASK_STACK_SIZE); 
}
