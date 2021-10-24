/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-08-03 21:27:12
 * @LastEditTime: 2021-08-05 23:15:31
 * @Description: 
 */

#include "LinOS.h"


/**
 * @Author: YangSL
 * @Description: 初始化互斥信号量
 * @param {tMutex *} mutex
 */
void vMutexInit(tMutex * mutex)
{
    vEventInit(&mutex->event, eEventTypeMutex);

    mutex->lockCount = 0;
    mutex->owner =(tTask *)0;
    mutex->ownerOriginalPrio = LinOS_PRI_COUNT;
    
}


/**
 * @Author: YangSL
 * @Description: 获取互斥量
 * @param {tMutex *} mutex      
 * @param {uint32_t} timeout    等待时间，为零则表示一直等待
 */
uint32_t uMutexTake(tMutex * mutex, uint32_t timeout)
{
    uint32_t status = uTaskEnterCritical();

    /* 判断互斥量是否又被使用 */
    if( mutex->lockCount <= 0)
    {
        /* 未被使用 */
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockCount++;

        vTaskExitCritical(status);
        return eErrorNoError;
    }
    else
    {
        /*互斥量已经被锁定 */
        /* 判断是否是被自己锁定的 */
        if(mutex->owner == currentTask)
        {
            mutex->lockCount++;

            vTaskExitCritical(status);
            return eErrorNoError;
        }
        else
        {
            /* 互斥量被其他任务占用了 */
            if(currentTask->prio < mutex->owner->prio)
            {
                tTask * owner = mutex->owner;

                /* 如果处以就绪列表中则：
                    1. 现将任务从就绪列表中移除
                    2. 提高拥有互斥量任务的优先级
                    3. 将提高后的拥有互斥量的任务重新加入优先级队列中*/
                if(owner->state == eReady)
                {
                    vTaskSchedUnRdy(owner);
                    owner->prio = currentTask->prio;
                    vTaskSchedRdy(owner);
                }
                else
                {
                    /* 其它状态，只需要修改优先级 */
                    owner->prio = currentTask->prio;
                }
            }

        /* 将当前任务加入等待队列中 */
        vEventWait(&mutex->event, currentTask, (void *)0, eEventTypeMutex, timeout);
        vTaskExitCritical(status);

        vTaskSched();
        return currentTask->waitEventResult;
        }
    }
}


/**
 * @Author: YangSL
 * @Description: 无等待获取互斥量，如果没有可用信号量，就立马返回
 * @param {tMutex *} mutex
 */
//uint32_t uMutexTryTake(tMutex * mutex)
//{

//}

/**
 * @Author: YangSL
 * @Description: 释放互斥量
 * @param {tMutex *} mutex
 */
uint32_t uMutexRelease(tMutex * mutex)
{
    uint32_t status = uTaskEnterCritical();

    if(mutex->lockCount <= 0)
    {
        vTaskExitCritical(status);
        return eErrorNoError;
    }

    if(mutex->owner != currentTask)
    {
        vTaskExitCritical(status);
        return eErrorOwner;
    }

    if(--mutex->lockCount != 0)
    {
        vTaskExitCritical(status);
        return eErrorNoError;
    }

    /* 是否有优先级继承发生 */
    if(mutex->ownerOriginalPrio != mutex->owner->prio)
    {
        /* 如果处以就绪列表中则：
            1. 现将任务从就绪列表中移除
            2. 将任务优先级恢复成之前的优先级
            3. 将提高后的拥有互斥量的任务重新加入优先级队列中*/
        if(mutex->owner->state == eReady)
        {
            vTaskSchedUnRdy(mutex->owner);
            currentTask->prio = mutex->ownerOriginalPrio;
            vTaskSchedRdy(mutex->owner);       
        }
        else
        {
            currentTask->prio = mutex->ownerOriginalPrio;
        }
    }

    /* 检查是否有任务等待 */
    if(uEventGetWaitCount(&mutex->event) > 0)
    {
        /* 优先唤醒位于队列头部的任务 */
        tTask * task = tEventWakeUp(&mutex->event, (void *)0, eErrorNoError);

        mutex->owner = task;
        mutex->ownerOriginalPrio = task->prio;
        mutex->lockCount++;

        /* 唤醒的任务如果比当前任务优先高，进行任务切换 */
        if(currentTask->prio < task->prio)
        {
            vTaskSched();
        }
    }

    vTaskExitCritical(status);
    return eErrorNoError;


}

 /**
 * @Author: YangSL
 * @Description: 删除该互斥量
 * @param {tMutex *} mutex
 */
uint32_t uMutexDelete(tMutex * mutex)
{
    uint32_t deleteCount = 0;
    uint32_t status = uTaskEnterCritical();

    /* 互斥量是否被使用了 */
    if(mutex->lockCount > 0)
    {
        if(mutex->owner->prio != mutex->ownerOriginalPrio)
        {
            /* 如果处以就绪列表中则：
                1. 现将任务从就绪列表中移除
                2. 将任务优先级恢复成之前的优先级
                3. 将提高后的拥有互斥量的任务重新加入优先级队列中*/
            if(mutex->owner->state == eReady)
            {
                vTaskSchedUnRdy(mutex->owner);
                currentTask->prio = mutex->ownerOriginalPrio;
                vTaskSchedRdy(mutex->owner);       
            }
            else
            {
                currentTask->prio = mutex->ownerOriginalPrio;
            }
        }

        deleteCount = uEventRemoveAll(&mutex->event, (void *)0, eErrorDelete);

        if(deleteCount > 0)
        {
            vTaskSched();
        }
    }

    vTaskExitCritical(status);
    return deleteCount;
}


/**
 * @Author: YangSL
 * @Description: 获取互斥量的相关信息
 * @param {tMutex *} mutex
 * @param {tMutexInfo *} info
 */
void vMutexGetInfo(tMutex * mutex,tMutexInfo * info )
{
    uint32_t status = uTaskEnterCritical();

   /*  拷贝需要的信息 */
    info->waitTaskCount = uEventGetWaitCount(&mutex->event);
    info->ownerPrio = mutex->ownerOriginalPrio;
    if (mutex->owner != (tTask *)0)
    {
        info->inheritedPrio = mutex->owner->prio;
    }
    else
    {
        info->inheritedPrio = LinOS_PRI_COUNT;
    }
    info->owner = mutex->owner;
    info->lockCount = mutex->lockCount;    


    vTaskExitCritical(status);
}

