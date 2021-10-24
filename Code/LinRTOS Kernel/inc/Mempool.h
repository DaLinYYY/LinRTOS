/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-31 10:37:03
 * @LastEditTime: 2021-07-31 14:06:17
 * @Description: 
 */

#ifndef _MEMPOOL_H
#define _MEMPOOL_H
#include "Event.h"


/*---------------------------------------------------------------------------------------------------------------------
 * MEMPOOL DEFINITIONS
 *--------------------------------------------------------------------------------------------------------------------*/
typedef struct _tMemPool
{
    /* 事件控制块 */
    tEvent event;

    /* 存储块的起始地址 */
    void * memStartAddress;

    /* 每个存储块的大小 */
    uint32_t blockSize;

    /* 存储块的总数 */
    uint32_t blockTotalCount;

    /* 未使用的存数块数量 */
    uint32_t blockFreeCount;

    /* 存储块的列表 */
    tList blockList;

    /* 挂起的列表 */
    tList suspendList;

}tMemPool;

typedef struct _MemPoolInfo
{
    /* 当前储存块计数 */
    uint32_t blockCount;

    /* 存储块总数 */
    uint32_t TalolCount;

    /* 未使用的存储块数量 */
    uint32_t freeCount;

    /* 每个存储块的大小 */
    uint32_t blockSize;

    /* 挂起的任务 */
    uint32_t suspendCount;
}tMemPoolInfo;

/*---------------------------------------------------------------------------------------------------------------------
 * MEMPOOL API
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 初始化存储控制块
 * @param {tMemPool *} memPool          等待初始化的存储控制块
 * @param {void*} startAddress          存储区的起始地址
 * @param {uint32_t} blockSize          每个块的大小
 * @param {uint32_t} blockTotalCount    总的块数量
 */
void vMemPoolInit(tMemPool * memPool, void* startAddress, uint32_t blockSize, uint32_t blockTotalCount);


/**
 * @Author: YangSL
 * @Description: 查询存储控制块的状态信息
 * @param {tMemPool *} memPool
 * @param {tMemPoolInfo *} info
 */
void vMemPoolGetInfo(tMemPool * memPool, tMemPoolInfo * info);


/**
 * @Author: YangSL
 * @Description: 有等待从内存池中分配存储块
 * @param {tMemPool *} memPool  内存池
 * @param {uint8_t **} memBlock 分配的内存块d地址
 * @param {uint32_t} timeout    超时时间
 */
uint32_t uMemPoolWaitAlloc(tMemPool * memPool, uint8_t ** memBlock,uint32_t timeout);

/**
 * @Author: YangSL
 * @Description: 无等待从内存池中分配存储块
 * @param {tMemPool *} memPool
 * @param {uint8_t **} memBlock
 */
uint32_t uMemPoolNoWaitAlloc(tMemPool * memPool, uint8_t ** memBlock);

/**
 * @Author: YangSL
 * @Description: 释放内存块
 * @param {tMemPool} *memPool   要操作的内存池
 * @param {void *} memBlock     释放内存块的地址
 */
void vMemPoolFree(tMemPool *memPool, void * memBlock);


/**
 * @Author: YangSL
 * @Description: 删除内存池
 * @param {tMemPool *} memPool
 */
uint32_t uMemPoolDelete(tMemPool * memPool);

#endif /* _MEMPOOL_H */


