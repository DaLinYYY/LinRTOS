/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-27 
 * @LastEditTime: 2021-07-27 21:57:07
 * @Description: 
 */

#include "Sem.h"
#include "LinOs.h"
/**
 * @Author: YangSL
 * @Description: 初始化信号量
 * @param {tSem} *sem
 * @param {uint32_t} startCount 初始的计数
 * @param {uint32_t} maxCount   最大计数，如果为0，则不限数量
 */
void vSemInit(tSem *sem, uint32_t startCount, uint32_t maxCount)
{
    vEventInit(&sem->event, eEventTypeSem);

	sem->maxCount = maxCount;
    if( maxCount == 0)
    {
        sem->count = startCount;
    }
    else
    {
        sem->count = (startCount > maxCount) ?maxCount :startCount;
    }
}


/**
 * @Author: YangSL
 * @Description: 获取信号量
 * @param {tSem *} sem      等待的信号量
 * @param {uint32_t} time   等待的时间，为零表示永远等待
 * RT-Thread描述：
 * 线程通过调用该函数获取信号量来获得信号量资源实例，当信号量值大于零时，
 * 线程将获得信号量， 并且相应的信号量值会减 1；如果信号量的值等于零，那么说明当前信号量资源实例不可用，
 * 申请该信号量的线程将根据 time 参数的情况选择直接返回、或挂起等待一段时间、或永久等待，直到其他线程或中断释放该信号量。
 */
uint32_t uSemTake(tSem * sem, uint32_t time)
{
    uint32_t status = uTaskEnterCritical();

    /* 当前信号量计数大于1，则直接减一处理 */
    if(sem->count > 0)
    {
        --sem->count;
        vTaskExitCritical(status);
        return eErrorNoError;
    }
    else /* 否则将当前任务加入等待队列 */
    {
        vEventWait(&sem->event, currentTask, (void *)0, eEventTypeSem, time);

        vTaskExitCritical(status);

        vTaskSched();

        return currentTask->waitEventResult;
    }
}



/**
 * @Author: YangSL
 * @Description: 无等待获取信号量
 * @param {tSem *} sem 等待的信号量
 */
uint32_t uSemTryTake(tSem * sem)
{
    uint32_t status = uTaskEnterCritical();

    if(sem->count > 0)
    {
        --sem->count;
        vTaskExitCritical(status);
        return eErrorNoError;       
    }
    else
    {
        vTaskExitCritical(status);
        return eErrorResourceUnavaliable;
    }
}


/**
 * @Author: YangSL
 * @Description: 释放信号量
 * @param {tSem *} sem 操作的信号量
 * RT-Thread描述：
 * 该函数将释放一个信号量，当信号量的值等于零时，并且有线程等待这个信号量时 
 * 释放信号量将唤醒等待在该信号量线程队列中的第一个线程，由它获取信号量
 * 否则 将把信号量的值加一
 */
void vSemRelease(tSem * sem)
{
    uint32_t status = uTaskEnterCritical();

    /* 有等待的任务 */
    if(uEventGetWaitCount(&sem->event) > 0)
    {
        tTask * task = tEventWakeUp(&sem->event, (void *)0, eErrorNoError);

        /* 如果等待的任务优先比当前任务高，需要进行任务切换 */
        if( task->prio < currentTask->prio)
        {
            vTaskSched();
        }
    }
    else /* 无等待任务 */
    {
        /* 计数加一 */
        ++sem->count;

        /* 如果计数不是无限的，同时当前计数大于最大设定值，则需要限幅处理 */
        if((sem->maxCount != 0) && (sem->count > sem->maxCount))
        {
            sem->count = sem->maxCount;
        }
    }
    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 删除信号量
 * @param {tSem *} sem
 */
uint32_t uSemDelete(tSem * sem)
{
    uint32_t status = uTaskEnterCritical();

    uint32_t count = uEventRemoveAll(&sem->event, (void *)0, eErrorDelete);

    sem->count = 0;

    vTaskExitCritical(status);

    if (count > 0)
    {
        vTaskSched();
    }

    return count;
}


/**
 * @Author: YangSL
 * @Description: 查询信号量的状态信息
 * @param {tSem *} sem
 * @param {tSemInfo *} info
 */
void vSemGetInfo(tSem * sem, tSemInfo * info)
{
    uint32_t status = uTaskEnterCritical();

    info->count = sem->count;
    info->maxCount = sem->maxCount;
    info->taskCount = uEventGetWaitCount(&sem->event);
    
    vTaskExitCritical(status);
}
