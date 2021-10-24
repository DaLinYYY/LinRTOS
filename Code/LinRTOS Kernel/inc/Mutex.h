/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-08-03 21:27:21
 * @LastEditTime: 2021-08-05 22:51:31
 * @Description: 
 */
#ifndef _MUTEX_H
#define _MUTEX_H

#include "Event.h"

/*---------------------------------------------------------------------------------------------------------------------
 * MUTEX DEFINITIONS
 *--------------------------------------------------------------------------------------------------------------------*/
typedef struct _tMutex
{
    /* 事件控制块 */
    tEvent event;

    /* 已被锁定的次数 */
    uint32_t lockCount;

    /* 拥有者 */
    tTask * owner;

    /* 拥有者原始的优先级 */
    uint32_t ownerOriginalPrio;

}tMutex;



typedef struct _tMutexInfo
{
    /* 等待该互斥量的任务数 */
    uint32_t waitTaskCount;
    /* 拥有该互斥量任务的优先级 */
    uint32_t ownerPrio;
    /* 拥有该互斥量任务继承的优先级 */
    uint32_t inheritedPrio;
    /* 互斥量锁定的次数 */
    uint32_t lockCount;
    /* 当前信号的拥有者 */
    tTask * owner;
}tMutexInfo;

/*---------------------------------------------------------------------------------------------------------------------
 * MUTEX API
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 初始化互斥信号量
 * @param {tMutex *} mutex
 */
void vMutexInit(tMutex * mutex);


/**
 * @Author: YangSL
 * @Description: 获取互斥量
 * @param {tMutex *} mutex
 * @param {uint32_t} timeout
 */
uint32_t uMutexTake(tMutex * mutex, uint32_t timeout);


/**
 * @Author: YangSL
 * @Description: 无等待获取互斥量，如果没有可用信号量，就立马返回
 * @param {tMutex *} mutex
 */
uint32_t uMutexTryTake(tMutex * mutex);


/**
 * @Author: YangSL
 * @Description: 释放互斥量
 * @param {tMutex *} mutex
 */
uint32_t uMutexRelease(tMutex * mutex);


/**
 * @Author: YangSL
 * @Description: 删除该互斥量
 * @param {tMutex *} mutex
 */
uint32_t uMutexDelete(tMutex * mutex);


/**
 * @Author: YangSL
 * @Description: 获取互斥量的相关信息
 * @param {tMutex *} mutex
 * @param {tMutexInfo *} info
 */
void vMutexGetInfo(tMutex * mutex,tMutexInfo * info );


#endif /* _MUTEX_H */
