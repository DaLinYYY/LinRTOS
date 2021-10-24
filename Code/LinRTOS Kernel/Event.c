/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-25
 * @LastEditTime: 2021-08-02 21:32:19
 * @Description: 
 */

#include "Event.h"
#include "LinOS.h"

/**
 * @Author: YangSL
 * @Description: 初始化事件控制块
 * @param {tEvent *} event  事件控制块
 * @param {eEventType} type 事件控制块的类型
 */
void vEventInit(tEvent * event, eEventType type)
{
    event->type = type;
    vListInit(&event->waitList);
}


/**
 * @Author: YangSL
 * @Description: 从事件控制块中唤醒首个等待的任务
 * @param {tEvent *} event      事件控制块
 * @param {tTask *} task        等待事件发生的任务
 * @param {void *} msg          事件消息存储的具体位置
 * @param {uint32_t} state      消息类型
 * @param {uint32_t} timeout    等待多长时间
 */
void vEventWait(tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout)
{
    /* 进入临界区 */
    uint32_t status = uTaskEnterCritical();

    // task->state = state;
    
    /* 设置任务等待的事件结构 */
    task->waitEvent = event;
    /* 设置任务等待事件的消息存储位置  */
    task->waitEventMsg = msg;
    /* 清空事件的等待结果 */
    task->waitEventResult = eErrorNoError;

    /* 将任务从就绪队列中移除 */
    vTaskRdyListRemove(task);

    /* 将任务插入到等待队列中 */
    vListInsertLast(&event->waitList, &task->linkNode);

    if (timeout)
    {
        tTaskDelayWait(task, timeout);
    }

    /* 退出临界区 */
    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 从事件控制块中唤醒最先等待的任务
 * @param {tEvent *} event  事件控制块
 * @param {void *} msg      事件消息
 * @param {uint32_t} result 告知事件的等待结果
 * @return value            首个等待的任务，如果没有任务等待，则返回0
 */
tTask * tEventWakeUp(tEvent * event, void * msg, uint32_t result)
{
    tTask * task  = (tTask *)0;
    tNode * node;

    uint32_t status = uTaskEnterCritical();

    /* 获取等待队列的第一个节点 */
    if((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
    {
        /* 获取该节点对应的任务控制块 */
        task = (tTask *)nodeParent(node, tTask, linkNode);

        /* 设置收到的消息、结构，清除相应的等待标志位 */
        task->waitEvent = (tEvent *)0;
        task->waitEventMsg = msg;
        task->waitEventResult = result;

        if(task->delayTicks != 0)
        {
            vTaskDelayWakeUp(task);
        }

        /* 将任务加入就绪队列 */
        vTaskSchedRdy(task);

    }

    vTaskExitCritical(status);

    return task;
}


/**
 * @Author: YangSL
 * @Description: 从事件控制块中唤醒指定任务
 * @param {tEvent *} eventv     事件控制块
 * @param {tTask *} task        等待唤醒的任务
 * @param {void} *msg           事件消息
 * @param {uint32_t} result     告知事件的等待结果
 */
void vEventWakeUpTask(tEvent * event, tTask * task, void *msg, uint32_t result)
{
    uint32_t status = uTaskEnterCritical();
    {
        vListRemoveNode(&event->waitList , &task->linkNode);

        /* 设置收到的消息、结构，清除相应的等待标志位 */
        task->waitEvent = (tEvent *)0;
        task->waitEventMsg = msg;
        task->waitEventResult = result;

        if(task->delayTicks != 0)
        {
            vTaskDelayWakeUp(task);
        }

        /* 将任务加入就绪队列 */
        vTaskSchedRdy(task);

    }
    vTaskExitCritical(status);
}

/**
 * @Author: YangSL
 * @Description: 将任务从其等待队列中强制移除
 * @param {tTask *} task    待移除的任务
 * @param {void *} msg      事件消息
 * @param {uint32_t} result 告知事件的等待结果
 */
void vEventRemoveTask(tTask * task, void * msg, uint32_t result)
{
 	 /* 进入临界区 */
    uint32_t status = uTaskEnterCritical();

	/* 将任务从所在的等待队列中移除 */
	vListRemoveNode(&task->waitEvent->waitList, &task->linkNode);

  	/* 设置收到的消息、结构，清除相应的等待标志位 */
    task->waitEvent = (tEvent *)0;
    task->waitEventMsg = msg;
   	task->waitEventResult = result;

	/* 退出临界区 */
    vTaskExitCritical(status);     
}


/**
 * @Author: YangSL
 * @Description: 清除所有等待中的任务，将事件发送给所有任务
 * @param {tEvent *} event  事件控制块
 * @param {void *} msg      事件消息
 * @param {uint32_t} result 告知事件的等待结果
 * @return value            唤醒的任务数量
 */
uint32_t uEventRemoveAll(tEvent * event, void * msg, uint32_t result)
{
    uint32_t  count;

    tNode * node ;

 	 /* 进入临界区 */
    uint32_t status = uTaskEnterCritical();    

    count = uGetListNodeCount(&event->waitList);

    while ((node = tListRemoveFirst(&event->waitList)) != (tNode * )0)
    {       
        tTask * task = (tTask *) nodeParent(node, tTask, linkNode);

        /* 设置收到的消息、结构，清除相应的等待标志位 */
        task->waitEvent = (tEvent *)0;
        task->waitEventMsg = msg;
        task->waitEventResult = result;

        if(task->delayTicks != 0)
        {
            vTaskDelayWakeUp(task);
        }

        /* 将任务加入就绪队列 */
        vTaskSchedRdy(task);
    }

	/* 退出临界区 */
    vTaskExitCritical(status);    

    return count; 
}


/**
 * @Author: YangSL
 * @Description: 事件控制块中等待的任务数量
 * @param {tEvent *} event
 */
uint32_t uEventGetWaitCount(tEvent * event)
{
    uint32_t count = 0;

    uint32_t status = uTaskEnterCritical();

    count = uGetListNodeCount(&event->waitList);  

    vTaskExitCritical(status);     

    return count;    
}
