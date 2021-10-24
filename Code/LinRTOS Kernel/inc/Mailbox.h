/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-28 
 * @LastEditTime: 2021-07-28 22:25:12
 * @Description: 
 */


#ifndef _MAILBIAX_H
#define _MAILBIAX_H

#include "Event.h"

#define defineMboxSendNormal    0x00
#define defineMboxSendFront     0x01

/*---------------------------------------------------------------------------------------------------------------------
 * MAINBOX DEFINITIONS
 *--------------------------------------------------------------------------------------------------------------------*/
typedef struct _tMailbox
{
    /* 事件控制块，包括：事件的列表和事件类型 */
    tEvent event;

    /* 当邮件的数量 */
    uint32_t count;

    /* 读取邮件的索引 */
    uint32_t read;

    /* 写邮件的索引 */
    uint32_t write;

    /* 最大允许的邮件数量 */
    uint32_t maxCount;

    /* 消息缓存区的 */
    void ** msgBuffer;

}tMbox;


typedef struct _tMboxInfo
{
	/* 当前的消息数量 */
    uint32_t count;

    /* 最大允许容纳的消息数量 */
    uint32_t maxCount;

    /* 当前等待的任务计数 */
    uint32_t taskCount;
}tMboxInfo;

/*---------------------------------------------------------------------------------------------------------------------
 * MAINBOX API
 *--------------------------------------------------------------------------------------------------------------------*/

/**
 * @Author: YangSL
 * @Description: 初始化邮箱
 * @param {tMbox} *mbox
 * @param {void **} msgBuffer
 * @param {uint32_t} maxCount
 */
void vMboxInit(tMbox *mbox, void ** msgBuffer, uint32_t maxCount);


/**
 * @Author: YangSL
 * @Description: 向邮箱中发送邮件
 * @param {tMbox *} mbox        获取消息的邮箱
 * @param {void **} msg         消息存储缓存区
 * @param {uint32_t} timeout    发送的选项
 */
uint32_t uMboxSend(tMbox * mbox, void * msg, uint32_t option);


/**
 * @Author: YangSL
 * @Description: 等待邮箱, 获取一则消息
 * @param {tMbox *} mbox     等待的邮箱
 * @param {void **} msg      消息存储缓存区
 * @param {uint32_t} timeout 超时时间
 */
uint32_t uMboxWait(tMbox * mbox, void ** msg, uint32_t timeout);


/**
 * @Author: YangSL
 * @Description: 清空邮箱中所有消息
 * @param {tMbox *} mbox
 */
void uMboxFlush(tMbox * mbox);


/**
 * @Author: YangSL
 * @Description: 删除邮箱
 * @param {tMbox} *mbox
 */
uint32_t uMboxDelete(tMbox *mbox);


/**
 * @Author: YangSL
 * @Description: 查询邮箱状态信息
 * @param {tMbox *} mbox
 * @param {tMboxInfo *} info
 */
void vMboxGetInfo (tMbox * mbox, tMboxInfo * info);
#endif /* _MAILBIAX_H */
