/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-08-01 21:40:56
 * @LastEditTime: 2022-01-29 11:36:53
 * @Description: 
 */
#include "LinOS.h"
#include "EventGroup.h"


/**
 * @Author: YangSL
 * @Description: 初始化事件标志组
 */
void vEventGroupInit(tEventGroup * eventGroup,  uint32_t flags)
{
    vEventInit(&eventGroup->event, eEventTypeEventGroup);

    eventGroup->flags = flags;
}


static uint32_t uEventGroupCheckAndConsume(tEventGroup * eventGroup, uint32_t type, uint32_t * flags)
{
    uint32_t srcFlags = * flags;

    /* 确定是设置操作还是清除操作 */
    uint32_t isSet = type & EVENTGROUP_SET;
    /* 确定是操作所有位还是部分bits */
    uint32_t isAll = type & EVENTGROUP_ALL;
    /* 是否消耗掉标志位 */
    uint32_t isConsume = type & EVENTGROUP_CONSUME;


	/* flagGroup->flags & flags:计算出哪些位为1
	  ~flagGroup->flags & flags:计算出哪位位为0  */
    uint32_t calcFlags = isSet ? (eventGroup->flags & srcFlags) : (~eventGroup->flags & srcFlags);

    if(((isAll != 0) && (calcFlags == srcFlags)) || ((isAll == 0) && (calcFlags != 0)))
    {
        /* 是否消耗掉标志位 */
        if(isConsume)
        {
            if(isSet)
            {
                /* 清除为1的标志位，变成0 */
                eventGroup->flags &= ~srcFlags;
            }
            else 
            {
                /* 清除为0的标志位，变成1 */
                eventGroup->flags |= srcFlags;
            }
        }

        *flags = calcFlags;
        return eErrorNoError;
    }
    *flags = calcFlags;
    return eErrorResourceUnavaliable;
}

/**
 * @Author: YangSL
 * @Description: 等待事件标志组中特定的标志
 * @param {tEventGroup *} eventGroup    等待的事件标志组
 * @param {uint32_t} waitType           等待的事件类型
 * @param {uint32_t} requestFlag        请求的事件标志
 * @param {uint32_t} *resultFlag        等待标志结果
 * @param {uint32_t} waitTicks          当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
 * @return : eErrorResourceUnavaliable.eErrorNoError,eErrorTimeout
 */
uint32_t uEventGroupWaitBits(tEventGroup * eventGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;

    uint32_t status = uTaskEnterCritical();

    result = uEventGroupCheckAndConsume(eventGroup, waitType, &flags);

    if(result != eErrorNoError)
    {
        /* 如果事件标志不满足条件，则插入到等待队列中 */
        currentTask->waitEventGroupType = waitType;
        currentTask->waitEventGroupFlag = requestFlag;
        vEventWait(&eventGroup->event, currentTask, (void *) 0, eEventTypeEventGroup, waitTicks);

        vTaskExitCritical(status);

        vTaskSched();

        *resultFlag = currentTask->waitEventGroupFlag;
        result =  currentTask->waitEventResult;
    }
    else
    {
        *resultFlag = flags;    
        vTaskExitCritical(status);   
    }
    
    return result;
}


/**
 * @Author: YangSL
 * @Description: 无等待的获取事件标志
 * @param {tEventGroup *} eventGroup
 * @param {uint32_t} waitType
 * @param {uint32_t} requestFlag
 * @param {uint32_t *} resultFlag
 * @return : eErrorResourceUnavaliable.eErrorNoError
 */
uint32_t uEventGroupNoWaitBits(tEventGroup * eventGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag)
{
    uint32_t flags = requestFlag;

    uint32_t status = uTaskEnterCritical();

    uint32_t result = uEventGroupCheckAndConsume(eventGroup, waitType, &flags);
    vTaskExitCritical(status);

    *resultFlag = flags;

    return result;
}


/**
 * @Author: YangSL
 * @Description: 设置相应的标志位置一或清零
 * @param {tEventGroup *} eventGroup
 * @param {uint8_t} isSet
 * @param {uint32_t} flags
 */
void vEventGroupNotify(tEventGroup * eventGroup, uint8_t isSet, uint32_t flags)
{

}


/**
 * @Author: YangSL
 * @Description: 设置清零的位
 * @param {tEventGroup *} eventGroup
 * @param {uint32_t} clearBits
 */
void vEventGroupClearBits(tEventGroup * eventGroup, uint32_t clearBits)
{
    uint32_t status = uTaskEnterCritical();
    {
        eventGroup->flags &= ~clearBits;
    }
    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 设置要置一的位
 * @param {tEventGroup} *eventGroup
 * @param {uint32_t} setBits
 */
void vEventGroupSetBits(tEventGroup * eventGroup, uint32_t setBits)
{

    tList * waitList;
    tNode * node;
    tNode * nextNode;
    uint8_t sched = 0;

    /* 临界代码 */
    uint32_t status = uTaskEnterCritical();
    {
        /* Set the bits. */
        eventGroup->flags |= setBits;

        waitList = &eventGroup->event.waitList;

        for(node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
        {
            uint32_t result;
            tTask * task = nodeParent(eventGroup, tTask, linkNode);
            uint32_t flags = task->waitEventGroupFlag;
            nextNode = node->nextNode;

            result = uEventGroupCheckAndConsume(eventGroup, task->waitEventGroupType, &flags);
            if(result == eErrorNoError)
            {
                task->waitEventGroupFlag = flags;
                vEventWakeUpTask(&eventGroup->event, task, (void * )0, eErrorNoError);
                sched = 1;
            }
        }

        if(sched)
        {
            vTaskSched();
        }
    }
    vTaskExitCritical(status);
}

