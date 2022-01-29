/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-08
 * @LastEditTime: 2022-01-29 15:53:37
 * @Description: 
 */
#include "Tasks.h"
#include "Bitmap.h"
#include "port.h"
#include "LinOS.h"
#include "LinOSConfig.h"

/* 当前任务：记录当前是哪个任务正在运行 */
tTask * currentTask;
/* 下一个将即运行的任务 */
tTask * nextTask;
/* 空闲任务 */
tTask * idleTask;

/* 所有任务的指针数组 */
tList taskPrioTable[LinOS_PRI_COUNT];
/* 调度锁计数器 */
uint8_t schedLockCount;
/* 延时队列 */
tList taskDelayList;
/* 优先级位置 */
Bitmap_t LinOSBitmap;


/* 空闲任务空间 */
tTask tTaskIdle;
tTaskStack idleTaskEnv[LinOS_IDLE_STACK_SIZE];


/* 空闲任务计数与最大计数 */
uint32_t idleCount;
uint32_t idleMaxCount;

/* 系统计数 */
long long systemTicks;

/* CPU使用率统计 */
static float cpuUsage;
/*---------------------------------------------------------------------------------------------------------------------
 * 任务调度相关操作
 *--------------------------------------------------------------------------------------------------------------------*/

/**
 * @Author: YangSL
 * @Description: 任务初始化API
 * @param {tTask *} task:       任务名称
 * @param {void} *              任务程序入口
 * @param {void} *param         传给任务的入口函数
 * @param {uint32_t *} stack    任务堆栈
 */
void vTaskInit (tTask * task, void (*entry)(void *), void *param, uint32_t prio, uint32_t * stack, uint32_t stackSize)
{
    /* 初始化该任务硬件相关的堆栈 */
    hwStackInit(task, entry, param, stack, stackSize);

    task->delayTicks            = 0;                                // 任务延时计数器
    task->prio                  = prio;                             // 设置优先级
    task->slice                 = LinOS_SLICE_MAX;                  // 初始化时间片计数
    task->supendCount           = 0;                                // 初始化挂起次数
	task->state         	    = eReady;

    task->clean                 = (void(*)(void *))0;           // 设置清理函数
    task->cleanParam            = (void *)0;                    // 设置传递给清理函数的参数
    task->requestsDeleteFlag    = 0;                            // 请求删除标记

    task->waitEvent             = (tEvent *)0;                  // 初始化等待事件
    task->waitEventMsg          = (void *) 0;                   // 初始化等待消息
    task->waitEventResult       = eErrorNoError;                // 初始化等待事件错误


    vNodeInit(&(task->delayNode));                              // 初始化延时结点
    
    vNodeInit(&(task->linkNode));                               // 初始化链接结点

    vTaskSchedRdy(task);                                        // 将任务插入就绪队列
}

void idleTaskEntry(void * parm);
/**
 * @Author: YangSL
 * @Description: 任务调度函数API
 */
void vTaskSched(void)
{
    tTask * tempTask;
    uint32_t status = uTaskEnterCritical();
    
    /* 创建空闲任务 */
    static uint32_t createIdleFlag = 0;
    if(createIdleFlag == 0)
    {
        /* 创建空闲任务 */
        vTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, LinOS_PRI_COUNT - 1,  idleTaskEnv, LinOS_STACK_SIZE);
        createIdleFlag = 1;
    }

    /* 判断调度器是否上锁 */
    if (schedLockCount > 0) 
    {
        vTaskExitCritical(status);
        return;
    }

    /* 寻找优先级最高的任务 */
    tempTask = tTaskHighestReady();
    {
        if (tempTask != currentTask)
        {
            nextTask = tempTask;
            taskScheduler();
        }
    }

    vTaskExitCritical(status);
}

static void checkCpuUsage(void);
/**
 * @Author: YangSL
 * @Description: 系统时钟节拍处理
 */
void vTaskSystemTickHandler (void) 
{
    tNode *node;

    /* 进入临界区 */
    uint32_t  status = uTaskEnterCritical();



    /* 系统计数加一 */
    systemTicks++;

    /* 检测CPU使用情况 */
#if LinOS_ENABLE_CPU_CHECK
    checkCpuUsage();
#endif /* LinOS_ENABLE_CPU_CHECK */

    /* 遍历所有的延时列表 */
    for (node = taskDelayList.headNode.nextNode; node != &(taskDelayList.headNode); node = node->nextNode)
    {
        tTask * task = nodeParent(node , tTask, delayNode);

        if(--task->delayTicks == 0)
        {
            /* 如果任务还处于等待事件的状态，则将其从事件等待队列中唤醒 */
            if (task->waitEvent) 
            {
                /* 此时，消息为空，等待结果为超时 */
                vEventRemoveTask(task, (void *)0, eErrorTimeout);
            }


            /* 将任务从延时队列中移除 */
            vTaskDelayWakeUp(task);

            /* j将任务添加到就绪列表 */
            vTaskSchedRdy(task);
        }
    }

    if(--currentTask->slice == 0)
    {
        if(uGetListNodeCount(&taskPrioTable[currentTask->prio]) > 0)
        {
            /* 刷新要运行的同一个优先级下的任务 */
            tListRemoveFirst(&taskPrioTable[currentTask->prio]);
            vListInsertLast(&taskPrioTable[currentTask->prio], &(currentTask->linkNode));

            /* 重置计数器 */
            currentTask->slice = LinOS_SLICE_MAX;
        }
    }

    /* 退出临界区 */
    vTaskExitCritical(status);

/* 是否使用定时器 */
#if LinOS_TIMER_SWITCH
    vTimerModulCheckTick();
#endif

    vTaskSched();
}


/*---------------------------------------------------------------------------------------------------------------------
 * 调度器开关
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 禁止任务调度
 */
void vTaskSchedDisable(void)
{
    uint32_t status = uTaskEnterCritical();

    if (schedLockCount <255){
        schedLockCount++;
    }
    
    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 使能任务调度器
 */
void vTaskSchedEnable(void)
{
    uint32_t status = uTaskEnterCritical();

    if(schedLockCount >0)
    {
        if(--schedLockCount == 0)
        {
            vTaskSched();
        }
    }
}


/**
 * @Author: YangSL
 * @Description: 调度器初始化
 */
void vTaskSchedInit(void)
{
	int i;
    schedLockCount = 0;

    vBitmapInit(&LinOSBitmap);
    for(i = 0; i < LinOS_PRI_COUNT; i++)
    {
        vListInit(&taskPrioTable[i]);
    }
}


/*---------------------------------------------------------------------------------------------------------------------
 * 任务相关队列操作  (就绪态)
 *--------------------------------------------------------------------------------------------------------------------*/

/**
 * @Author: YangSL
 * @Description: 将任务加入就绪列表
 * @param {tTask *} task
 */
void vTaskSchedRdy(tTask * task)
{
    vListInsertLast(&taskPrioTable[task->prio], &(task->linkNode));
    
    vBitmapSet(&LinOSBitmap, task->prio);
}

/**
 * @Author: YangSL
 * @Description: 将任务从就绪列表中移除
 * @param {tTask *} task
 */
void vTaskSchedUnRdy(tTask * task)
{
    /*  */
    vListRemoveNode(&taskPrioTable[task->prio], &(task->linkNode));

    if(uGetListNodeCount(&taskPrioTable[task->prio]) == 0)
    {
        vBitmapClear(&LinOSBitmap, task->prio);
    }
    
}

/**
 * @Author: YangSL
 * @Description: 获取当前最高优先级且可运行的任务
 */
tTask *tTaskHighestReady(void)
{
    /* 获取最高优先级 */
    uint32_t highestPrio = uBitmapGetFirstSet(&LinOSBitmap);
    /* 获得最高优先级对应列表的第一个节点 */
    tNode * node = tGetFirstNode(&taskPrioTable[highestPrio]);
    /* 返回对应任务的tList地址 */
    return (tTask *)nodeParent(node, tTask, linkNode);
}


/**
 * @Author: YangSL
 * @Description: 将任务从就绪列表中移除
 * @param {tTask *} task
 */
void vTaskRdyListRemove(tTask * task)
{
    /*  */
    vListRemoveNode(&taskPrioTable[task->prio], &(task->linkNode));

    if(uGetListNodeCount(&taskPrioTable[task->prio]) == 0)
    {
        vBitmapClear(&LinOSBitmap, task->prio);
    }
    
}
/*---------------------------------------------------------------------------------------------------------------------
 * 任务相关队列操作  (阻塞态)
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 延时队列初始化
 */
void vTaskDelayInit(void)
{
    vListInit(&taskDelayList);
}

/**
 * @Author: YangSL
 * @Description: 将任务加入延时队列
 * @param {tTask *} task    : 延时列表
 * @param {uint32_t} ticks  : 延时时间
 */
void tTaskDelayWait(tTask * task, uint32_t ticks)
{   
    task->delayTicks = ticks;
    vListInsertLast(&taskDelayList, &(task->delayNode));
    task->state = eBlocked;
}


/**
 * @Author: YangSL
 * @Description: 将延时的任务从延时队列中唤醒
 * @param {tTask *} task
 */
void vTaskDelayWakeUp(tTask * task)
{
    vListRemoveNode(&taskDelayList, &(task->delayNode));
    task->state = eReady;
}


/**
 * @Author: YangSL
 * @Description: 将延时的任务从延时队列中移除
 * @param {tTask *} task
 */
void vTaskDelayListRemove(tTask * task)
{
    vListRemoveNode(&taskDelayList, &(task->delayNode));
}
/*---------------------------------------------------------------------------------------------------------------------
 * 任务相关队列操作  (挂起态)
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 挂起指定的任务
 * @param {tTask *} task : 指定任务
 */
void vTaskSupend(tTask * task)
{
    uint32_t status = uTaskEnterCritical();

    if(task->state != eBlocked)
    {
        /* 如果是该任务是第一次挂起，则需要以下处理
            1. 将任务状态设置为挂起状态
            2. 从就绪列表中移除 */
        if( ++task->supendCount <= 1)
        {
            task->state = eSuspended;

            vTaskSchedUnRdy(task);
            
            /* 如果挂起的任务是当前正在运行的，则立马就进行调度 */
            if(task == currentTask)
            {
                vTaskSched();
            }
        }
    }
    vTaskExitCritical(status);

}


/**
 * @Author: YangSL
 * @Description: 将任务从挂起态唤醒
 * @param {tTask *} task
 */
void vTaskResume(tTask * task)
{
    uint32_t status = uTaskEnterCritical();

    if(task->state == eSuspended)
    {
        if(--task->supendCount == 0)
        {
            /* 将任务设置为就绪态 */
            task->state = eReady;

            /* 添加到就绪列表 */
            vTaskSchedRdy(task);

            /* 可能唤醒的任务优先级比当前高，就需要执行一次任务调度 */
            vTaskSched();
        }
    }

    vTaskExitCritical(status);
}


/*---------------------------------------------------------------------------------------------------------------------
 * 任务相关队列操作  (删除)
 *--------------------------------------------------------------------------------------------------------------------*/

/**
 * @Author: YangSL
 * @Description: 设置任务被删除时调用的清理回调函数
 * @param {tTask *} task  :待设置的任务
 * @param {void} *param   :清理函数入口地址
 * @param {void *} param  :传递给清理函数的参数
 */
void vTaskSetCleanCallback(tTask * task, void (*clean )(void *param), void * param)
{
    task->clean = clean;
    task->cleanParam = param;    
}


/**
 * @Author: YangSL
 * @Description: 强制删除任务
 * @param {tTask *} task
 */
void vTaskForceDelete(tTask * task)
{  
    /* 进入临界区 */
    uint32_t status = uTaskEnterCritical();

    /*  如果任务处于延时状态，则从延时队列中删除 */
    if ( task->state == eBlocked)
    {
        vTaskDelayListRemove(task);
    }
    /*  如果任务不处于挂起状态，那么就是就绪态，从就绪表中删除 */
    else if(!(task->state == eSuspended))
    {
        vTaskRdyListRemove(task);
    }

    /* 删除时，如果有设置清理函数，则调用清理函数 */
    if (task->clean) 
    {
        task->clean(task->cleanParam);
    }

    /* 如果删除的是自己，那么需要切换至另一个任务，所以执行一次任务调度 */
    if (currentTask == task) 
    {
        vTaskSched();
    }

    /* 退出临界区 */
    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 请求删除某个任务，最后由任务自己决定是否删除自己
 * @param {tTask *} task
 */
void vTaskRequestsDelete(tTask * task)
{
    /* 进入临界区 */
    uint32_t status = uTaskEnterCritical();

    /* 设置清除删除标记 */
    task->requestsDeleteFlag = 1;

    /* 退出临界区 */
    vTaskExitCritical(status);
}

/**
 * @Author: YangSL
 * @Description: 检查是否已经被请求删除自己
 * @param {tTask *} task
 */
uint8_t uTaskCheckDelete(void)
{
    uint8_t deleteFlag;

    /* 进入临界区 */
    uint32_t status = uTaskEnterCritical();

    deleteFlag = currentTask->requestsDeleteFlag;

    /* 退出临界区 */
    vTaskExitCritical(status);    
	
	return deleteFlag;
}

/**
 * @Author: YangSL
 * @Description: 删除自己
 */
void vTaskDeleteSelf(void)
{
    /* 进入临界区 */
    uint32_t status = uTaskEnterCritical();

    /* 将自身从就绪队列中移除 */
    vTaskRdyListRemove(currentTask);

    /* 调用清理函数 */
    if(currentTask->clean)
    {
        currentTask->clean(currentTask->cleanParam);
    }

    vTaskSched();

    /* 退出临界区 */
    vTaskExitCritical(status);  
}

/*---------------------------------------------------------------------------------------------------------------------
 * Others
 *--------------------------------------------------------------------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: RTOS延时函数
 * @param {uint32_t} delay
 */
void vTaskDelay(uint32_t delay)
{
    uint32_t status = uTaskEnterCritical();

    /* 设置延时值，插入延时队列 */
    tTaskDelayWait(currentTask, delay);

    /* 将任务从就绪表中移除 */
    vTaskSchedUnRdy(currentTask);

    vTaskSched();

    vTaskExitCritical(status);
}


/**
 * @Author: YangSL
 * @Description: 获取任务相关信息
 * @param {tTask *} task   ：需要查询的任务
 * @param {tTaskInfo} info ：任务信息存储结构
 */
void vTaskGetInfo(tTask * task, tTaskInfo *info)
{
    uint32_t * stackEnd;

    /* 进入临界区 */
    uint32_t status = uTaskEnterCritical();

    /* 延时信息 */
    info->delayTicks = task->delayTicks;

    /* 优先级 */               
    info->prio = task->prio;        

    /* 任务状态 */                    
    info->state = task->state;                         

    /* 剩余时间片 */
    info->slice = task->slice;                         
    
    /* 被挂起的次数 */
    info->supendCount = task->supendCount;          

    info->stackSize = task->stackSize;

    /* 计算堆桟使用情况 */  
    info->stackFree = 0;
    stackEnd = task->stackBase;
    /* 注意堆桟是从上往下增长的->stackEnd最小 */
    while((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(tTaskStack)))
    {
        info->stackFree++;
    }

    /* 转换成字节数 */
    info->stackFree *= sizeof(tTaskStack);

    /* 退出临界区 */
    vTaskExitCritical(status);     
}



/**
 * @Author: YangSL
 * @Description: 空闲任务
 * @param {void *} parm
 */
void idleTaskEntry(void * parm)
{
    uint32_t status = uTaskEnterCritical();

#if LinOS_TIMER_SWITCH
    /* 初始化定时器相关资源 */
    vTimerModulInit();
    /* 创建定时器任务 */
    vTimerTickInit();
#endif

    vTaskExitCritical(status);

    for(;;)
	{
		uint32_t status = uTaskEnterCritical();
		idleCount++;
		vTaskExitCritical(status);
	}
}


static void checkCpuUsage(void)
{
    if(systemTicks %  1000 == 0)
    {
        idleMaxCount = idleCount;

        cpuUsage = 100 - (idleCount / 1000) * 100;

        idleCount = 0;
    }   
}

/**
 * @Author: YangSL
 * @Description: 获取CPU使用率
 */
float fTaskGetCpuUsage(void) 
{
    float usage = 0;

    uint32_t status = uTaskEnterCritical();
    usage = cpuUsage;
    vTaskExitCritical(status);

    return usage;    
}
