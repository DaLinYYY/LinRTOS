/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-27
 * @LastEditTime: 2021-07-27 21:51:29
 * @Description: 
 */

#ifndef _SEM_H
#define _SEM_H

#include "Event.h"

/*---------------------------------------------------------------------------------------------------------------------
 * SEM DEFINITIONS
 *--------------------------------------------------------------------------------------------------------------------*/

typedef struct  _tSem
{
    /* 事件控制块 */
    tEvent event;

    /* 当前的计数 */
    uint32_t count;

    /* 最大计数 */
    uint32_t maxCount;
	
}tSem;

typedef struct  _eSemInfo
{
    /* 当前信号量计数 */
    uint32_t count;
    /* 信号量允许的最大计数 */
    uint32_t maxCount;
    /* 当前等待任务的计数 */
    uint32_t taskCount;
}tSemInfo;



/*---------------------------------------------------------------------------------------------------------------------
 * SEM API
 *--------------------------------------------------------------------------------------------------------------------*/

/**
 * @Author: YangSL
 * @Description: 初始化信号量
 * @param {tSem} *sem
 * @param {uint32_t} startCount 初始的计数
 * @param {uint32_t} maxCount   最大计数，如果为0，则不限数量
 */
void vSemInit(tSem *sem, uint32_t startCount, uint32_t maxCount);


/**
 * @Author: YangSL
 * @Description: 获取信号量
 * @param {tSem *} sem      等待的信号量
 * @param {uint32_t} time   等待的时间，为零表示永远等待
 */
uint32_t uSemTake(tSem * sem, uint32_t time);


/**
 * @Author: YangSL
 * @Description: 无等待获取信号量
 * @param {tSem *} sem 等待的信号量
 */
uint32_t uSemTryTake(tSem * sem);


/**
 * @Author: YangSL
 * @Description: 释放信号量
 * @param {tSem *} sem 操作的信号量
 */
void vSemRelease(tSem * sem);


/**
 * @Author: YangSL
 * @Description: 删除信号量
 * @param {tSem *} sem
 */
uint32_t uSemDelete(tSem * sem);


/**
 * @Author: YangSL
 * @Description: 查询信号量的状态信息
 * @param {tSem *} sem
 * @param {tSemInfo *} info
 */
void vSemGetInfo(tSem * sem, tSemInfo * info);

#endif /* _SEM_H */


