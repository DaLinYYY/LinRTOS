/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-08-01 21:40:39
 * @LastEditTime: 2021-08-02 21:39:31
 * @Description: 
 */

#ifndef _EVENT_GROUP_H
#define _EVENT_GROUP_H

#include "Event.h"

/*---------------------------------------------------------------------------------------------------------------------
 * FLAGGROUP DEFINITIONS
 *--------------------------------------------------------------------------------------------------------------------*/

typedef struct _tEventGroup
{
    /* 事件控制块 */
    tEvent event;

    /* 事件标志 */
    uint32_t flags;
    
}tEventGroup;

#define EVENTGROUP_CLEAR        (0X0 << 0)
#define EVENTGROUP_SET          (0X1 << 0)
#define EVENTGROUP_ANY          (0X0 << 1)
#define EVENTGROUP_ALL          (0X1 << 1)

#define EVENTGROUP_SET_ALL      (EVENTGROUP_SET | EVENTGROUP_ALL)
#define EVENTGROUP_SET_ANY      (EVENTGROUP_SET | EVENTGROUP_ANY)
#define EVENTGROUP_CLEAR_ALL    (EVENTGROUP_CLEAR | EVENTGROUP_ALL)
#define EVENTGROUP_CLEAR_ANY    (EVENTGROUP_CLEAR | EVENTGROUP_ANY)

#define EVENTGROUP_CONSUME      (0X1 << 7)
/*---------------------------------------------------------------------------------------------------------------------
 * FLAGGROUP API
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 初始化事件标志组
 */
void vEventGroupInit(tEventGroup * eventGroup,  uint32_t flags);


/**
 * @Author: YangSL
 * @Description: 等待事件标志组中特定的标志
 * @param {tEventGroup *} eventGroup  等待的事件标志组
 * @param {uint32_t} waitType       等待的事件类型
 * @param {uint32_t} requestFlag    请求的事件标志
 * @param {uint32_t} *resultFlag    等待标志结果
 * @param {uint32_t} waitTicks      当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
 * @return : eErrorResourceUnavaliable.eErrorNoError,eErrorTimeout
 */
uint32_t uEventGroupWaitBits(tEventGroup * eventGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag, uint32_t waitTicks);


/**
 * @Author: YangSL
 * @Description: 无等待的获取事件标志
 * @param {tEventGroup *} eventGroup
 * @param {uint32_t} waitType
 * @param {uint32_t} requestFlag
 * @param {uint32_t *} resultFlag
 * @return : eErrorResourceUnavaliable.eErrorNoError
 */
uint32_t uEventGroupNoWaitBits(tEventGroup * eventGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag);


/**
 * @Author: YangSL
 * @Description: 
 * @param {tEventGroup *} eventGroup
 * @param {uint8_t} isSet
 * @param {uint32_t} flags
 */
void vEventGroupNotify(tEventGroup * eventGroup, uint8_t isSet, uint32_t flags);


/**
 * @Author: YangSL
 * @Description: 设置清零的位
 * @param {tEventGroup *} eventGroup
 * @param {uint32_t} clearBits
 */
void vEventGroupClearBits(tEventGroup * eventGroup, uint32_t clearBits);


/**
 * @Author: YangSL
 * @Description: 设置要置一的位
 * @param {tEventGroup} *eventGroup
 * @param {uint32_t} setBits
 */
void vEventGroupSetBits(tEventGroup * eventGroup, uint32_t setBits);

#endif /* _FLAG_GROUP_H */
