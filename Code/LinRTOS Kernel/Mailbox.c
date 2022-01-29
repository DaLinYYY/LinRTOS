/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-28
 * @LastEditTime: 2022-01-29 11:38:21
 * @Description: 
 */

#include "Mailbox.h"
#include "LinOS.h"
/*---------------------------------------------------------------------------------------------------------------------
 * MAINBOX METHODS
 *--------------------------------------------------------------------------------------------------------------------*/


/**
 * @Author: YangSL
 * @Description: 初始化邮箱
 * @param {tMbox} *mbox
 * @param {void **} msgBuffer
 * @param {uint32_t} maxCount
 */
void vMboxInit(tMbox *mbox, void ** msgBuffer, uint32_t maxCount)
{
    vEventInit(&mbox->event, eEventTypeMbox);

    mbox->msgBuffer = msgBuffer;
    mbox->maxCount = maxCount;
    mbox->count = 0;
    mbox->read  = 0;
    mbox->write = 0;
}


/**
 * @Author: YangSL
 * @Description: 向邮箱中发送邮件
 * @param {tMbox *} mbox        获取消息的邮箱
 * @param {void **} msg         消息存储缓存区
 * @param {uint32_t} option     插入的位置：队列前: defineMboxSendFront 队列末尾 : defineMboxSendNormal
 */
uint32_t uMboxSend(tMbox * mbox, void * msg, uint32_t option)
{
    uint32_t status = uTaskEnterCritical();

    /* 判断是否有等待邮件的任务 */
    if( uEventGetWaitCount(&mbox->event) > 0)
    {
        /* 如果有任务等待，就唤醒等待队列的第一个任务
            唤醒之后需要判断唤醒的任务优先级与当前任务优先级关系，如果当前任务优先级低于唤醒的任务，需要任务切换 */
        tTask * task = tEventWakeUp(&mbox->event, (void *)msg, eErrorNoError);

        if( currentTask->prio < task->prio)
            vTaskSched();
    }
    /* 没有等待的任务 */
    else
    {
        /* 如果邮件消息的数量已经达到的最大值，就返回相应错误 */
        if (mbox->count >= mbox->maxCount)
        {
            vTaskExitCritical(status);
            return eErrorResourceFull;
        }
        else
        {
            /* 将任务插入到队列最前面，这样可以让该任务最先获取到邮件消息*/
            if(option & defineMboxSendFront)
            {
                if(mbox->read <= 0)
                {
                    mbox->read = mbox->maxCount - 1;
                }
                else
                {
                    --mbox->read;
                }
                mbox->msgBuffer[mbox->read] = msg;
            }
            /* 正常的插入到队列末尾 */
            else
            {
                mbox->msgBuffer[mbox->write++] = msg;
                if(mbox->write >= mbox->maxCount)
                {
                    mbox->write = 0;
                }       
            }

        }
        mbox->count++;
    }

    vTaskExitCritical(status);
    return eErrorNoError;
}


/**
 * @Author: YangSL
 * @Description: 等待邮箱, 获取一则消息
 * @param {tMbox *} mbox     等待的邮箱
 * @param {void **} msg      消息存储缓存区
 * @param {uint32_t} timeout 超时时间
 */
uint32_t uMboxWait(tMbox * mbox, void ** msg, uint32_t timeout)
{  
    uint32_t status = uTaskEnterCritical();

    /* 判断邮箱是否有消息 */
    if (mbox->count > 0)
    {
        /* 邮箱不为空则：
            1. 将邮箱消息计数值减一
            2. 从read位置获取消息信息
            3. 将read向后移动一位并判断read是否越界 */
        mbox->count--;

        *msg = mbox->msgBuffer[mbox->read];

        mbox->read++;

        if(mbox->read >= mbox->maxCount)
        {
            mbox->read = 0;
        }
		vTaskExitCritical(status);
		return eErrorNoError;
    }
    else
    {
        /* 邮箱为空则将当前任务加入等待队列，并设置超时时间 */
        vEventWait(&mbox->event, currentTask, (void *)0, 0, timeout);
        vTaskExitCritical(status);

        vTaskSched();

        /* 等待调度回来获取信息 */
        *msg = currentTask->waitEventMsg;

        return currentTask->waitEventResult;
    }
}


/**
 * @Author: YangSL
 * @Description: 清空邮箱中所有消息
 * @param {tMbox *} mbox
 */
void uMboxFlush(tMbox * mbox)
{
    uint32_t status = uTaskEnterCritical();

    if(uEventGetWaitCount(&mbox->event) == 0)
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }

    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 删除邮箱
 * @param {tMbox} *mbox
 */
uint32_t uMboxDelete(tMbox *mbox)
{
    uint32_t status = uTaskEnterCritical();

    uint32_t count = uEventRemoveAll(&mbox->event, (void *)0, eErrorDelete);

    vTaskExitCritical(status);

    if (count > 0)
    {
        vTaskSched();
    }

    return count;
}

/**
 * @Author: YangSL
 * @Description: 查询邮箱状态信息
 * @param {tMbox *} mbox
 * @param {tMboxInfo *} info
 */
void vMboxGetInfo (tMbox * mbox, tMboxInfo * info)
{
    uint32_t status = uTaskEnterCritical();

    /* 拷贝需要的信息 */
    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = uEventGetWaitCount(&mbox->event);

    vTaskExitCritical(status);
}

