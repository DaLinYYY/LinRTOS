/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-31 10:37:13
 * @LastEditTime: 2021-07-31 14:06:13
 * @Description: 
 */

#include "Mempool.h"
#include "LinOS.h"

/**
 * @Author: YangSL
 * @Description: 初始化存储控制块
 * @param {tMemPool *} memPool          等待初始化的存储控制块
 * @param {void*} startAddress          存储区的起始地址
 * @param {uint32_t} blockSize          每个块的大小
 * @param {uint32_t} blockTotalCount    总的块数量
 */
void vMemPoolInit(tMemPool * memPool, void* startAddress, uint32_t blockSize, uint32_t blockTotalCount)
{
    uint8_t * memBlockStart = (uint8_t *) startAddress;
    uint8_t * memBlockEnd   = (uint8_t *) startAddress + blockSize * blockTotalCount;

    if(blockSize < sizeof(tNode))
    {
        return;
    }

    /* 初始化事件控制块 */
    vEventInit(&memPool->event, eEventTypeMemPool);

    /* 初始化Block相关参数 */
    memPool->memStartAddress = startAddress;
    memPool->blockSize = blockSize;
    memPool->blockTotalCount = blockTotalCount;

    /* 初始化存储块链表 */
    vListInit(&memPool->blockList);
    while(memBlockStart < memBlockEnd)
    {
        /* 初始化每一个储存块节点 */
        vNodeInit((tNode *)memBlockStart);
        /* 将存储块节点加入到储存块链表中 */
        vListInsertLast(&memPool->blockList, (tNode *)memBlockStart);
        memBlockStart += blockSize;
    }

    /* 初始化因该存储块挂起的任务链表 */
    vListInit(&memPool->suspendList);
}


/**
 * @Author: YangSL
 * @Description: 查询存储控制块的状态信息
 * @param {tMemPool *} memPool
 * @param {tMemPoolInfo *} info
 */
void vMemPoolGetInfo(tMemPool * memPool, tMemPoolInfo * info)
{
    uint32_t status = uTaskEnterCritical();

    info->blockCount = memPool->blockList.nodeCount;
    info->TalolCount = memPool->blockTotalCount;
    info->freeCount  = info->TalolCount - info->blockCount;
    info->blockSize  = memPool->blockSize;
    info->suspendCount = memPool->suspendList.nodeCount;

    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 有等待从内存池中分配存储块
 * @param {tMemPool *} memPool  内存池
 * @param {uint8_t **} memBlock 分配的内存块d地址
 * @param {uint32_t} timeout    超时时间
 */
 uint32_t uMemPoolWaitAlloc(tMemPool * memPool, uint8_t ** memBlock,uint32_t timeout)
{
    uint32_t status =uTaskEnterCritical();

    /* 如果内存是中还有空余的内存块，直接取出一块 */
    if(uGetListNodeCount(&memPool->blockList) > 0)
    {
        *memBlock = (uint8_t *)tListRemoveFirst(&memPool->blockList);
        vTaskExitCritical(status);
        return eErrorNoError;
    }
    /* 没有空余的内存块 */
    else
    {   
        /* 将任务加入事件的队列中 */
        vEventWait(&memPool->event, currentTask, (void *)0, eEventTypeMemPool, timeout);
        vTaskExitCritical(status);

        /* 发起任务调度 */
        vTaskSched();

        /* 当任务切换回来时获取任务的消息 */
        *memBlock = currentTask->waitEventMsg;

        /* 返回事件结果 */
        return currentTask->waitEventResult;
    }

    vTaskExitCritical(status);
}

/**
 * @Author: YangSL
 * @Description: 无等待从内存池中分配存储块
 * @param {tMemPool *} memPool
 * @param {uint8_t **} memBlock
 */
uint32_t uMemPoolNoWaitAlloc(tMemPool * memPool, uint8_t ** memBlock)
{
    uint32_t status = uTaskEnterCritical();

    if(uGetListNodeCount(&memPool->blockList) > 0)
    {
        *memBlock = (uint8_t *)tListRemoveFirst(&memPool->blockList);
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
 * @Description: 释放内存块
 * @param {tMemPool} *memPool   要操作的内存池
 * @param {void *} memBlock     释放内存块的地址
 */
void vMemPoolFree(tMemPool *memPool, void * memBlock)
{
    uint32_t status = uTaskEnterCritical();

    /* 检查是否有等待请求储存块的任务 */
    if(uEventGetWaitCount(&memPool->event) > 0)
    {
        /* 如果有直接唤醒等待队列的第一个任务 */
        tTask * task = tEventWakeUp(&memPool->event, (void *)memBlock, eErrorNoError);

        /* 检查是或需要任务切换 */
        if(task->prio > currentTask->prio)
        {
            vTaskSched();
        }
    }
    else
    {
        /* 等待列表为空，则将存储块加入存储池中 */
        vListInsertLast(&memPool->blockList, (tNode *)memBlock);
    }

    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 删除内存池
 * @param {tMemPool *} memPool
 */
uint32_t uMemPoolDelete(tMemPool * memPool)
{
    uint32_t status = uTaskEnterCritical();

    /* 唤醒等待该内存块的所有任务 */
    uint32_t count = uEventRemoveAll(&memPool->event, (void *) 0, eErrorDelete);

    vTaskExitCritical(status);

    if (count)
    {
        vTaskSched();
    }
    return count;
}

