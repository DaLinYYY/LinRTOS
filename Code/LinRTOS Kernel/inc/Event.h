/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-25 
 * @LastEditTime: 2021-08-03 22:17:03
 * @Description: 
 */

#ifndef _EVENT_H
#define _EVENT_H

#include "Tasks.h"

/*---------------------------------------------------------------------------------------------------------------------
 * EVENTDEFINITIONS
 *--------------------------------------------------------------------------------------------------------------------*/
typedef enum _eEventType
{
    eEventTypeUnknown               = (0 << 16),    //未定义类型
    eEventTypeSem                 	= (1 << 16),    //信号量类型
    eEventTypeMbox                	= (2 << 16),    //邮箱类型
    eEventTypeMemPool              	= (3 << 16),    //存储块类型
    eEventTypeEventGroup           	= (4 << 16),    //事件标志组
    eEventTypeMutex                 = (5 << 16),    //互斥信号量类型
}eEventType;


typedef struct  _tEvent
{
    eEventType type;

    tList waitList; 
    
}tEvent;


/*---------------------------------------------------------------------------------------------------------------------
 * EVENT API
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 初始化事件控制块
 * @param {tEvent *} event  事件控制块
 * @param {eEventType} type 事件控制块的类型
 */
void vEventInit(tEvent * event, eEventType type);


/**
 * @Author: YangSL
 * @Description: 从事件控制块中唤醒首个等待的任务
 * @param {tEvent *} event      事件控制块
 * @param {tTask *} task        等待事件发生的任务
 * @param {void *} msg          事件消息存储的具体位置
 * @param {uint32_t} state      消息类型
 * @param {uint32_t} timeout    等待多长时间
 */
void vEventWait(tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout);


/**
 * @Author: YangSL
 * @Description: 从事件控制块中唤醒最先等待的任务
 * @param {tEvent *} event  事件控制块
 * @param {void *} msg      事件消息
 * @param {uint32_t} result 告知事件的等待结果
 * @return value            首个等待的任务，如果没有任务等待，则返回0
 */
tTask * tEventWakeUp(tEvent * event, void * msg, uint32_t result);


/**
 * @Author: YangSL
 * @Description: 从事件控制块中唤醒指定任务
 * @param {tEvent *} eventv     事件控制块
 * @param {tTask *} task        等待唤醒的任务
 * @param {void} *msg           事件消息
 * @param {uint32_t} result     告知事件的等待结果
 */
void vEventWakeUpTask(tEvent * event, tTask * task, void *msg, uint32_t result);


/**
 * @Author: YangSL
 * @Description: 将任务从其等待队列中强制移除
 * @param {tTask *} task    待移除的任务
 * @param {void *} msg      事件消息
 * @param {uint32_t} result 告知事件的等待结果
 */
void vEventRemoveTask(tTask * task, void * msg, uint32_t result);


/**
 * @Author: YangSL
 * @Description: 清除所有等待中的任务，将事件发送给所有任务
 * @param {tEvent *} event  事件控制块
 * @param {void *} msg      事件消息
 * @param {uint32_t} result 告知事件的等待结果
 * @return value            唤醒的任务数量
 */
uint32_t uEventRemoveAll(tEvent * event, void * msg, uint32_t result);

/**
 * @Author: YangSL
 * @Description: 事件控制块中等待的任务数量
 * @param {tEvent *} event
 */
uint32_t uEventGetWaitCount(tEvent * event);

#endif /* _EVENT_H */
