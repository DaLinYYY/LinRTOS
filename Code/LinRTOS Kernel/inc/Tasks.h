/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-06 
 * @LastEditTime: 2022-01-29 15:53:57
 * @Description: 
 */
#ifndef INC_TASK_H
#define INC_TASK_H

#include <stdint.h>
#include "Bitmap.h"
#include "Lists.h"
#include "LinOSConfig.h"
/*---------------------------------------------------------------------------------------------------------------------
 * TASK MACROS
 *--------------------------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------*/

	

/* 获取结点所在的父struct结构首地址 */
// #define nodeParent(node, parent, name) (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name)
#define nodeParent(node, parent, name) (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name)
/*---------------------------------------------------------------------------------------------------------------------
 * TASK DEFINITIONS
 *--------------------------------------------------------------------------------------------------------------------*/



/* 任务的状态定义 */
typedef  enum
{
	eRunning = 0,	/* 运行状态. */
	eReady,			/* 就绪状态. */
	eBlocked,		/* 阻塞状态. */
	eSuspended,		/* 挂起状态. */    
    eDeleted,       /* 删除状态  */
}eTaskState;


/* Cortex-M的堆栈单元类型：堆栈单元的大小为32位，所以使用uint32_t */
typedef uint32_t tTaskStack;

// 前置声明
struct _tEvent;

 /* 任务结构：包含了一个任务的所有信息 */
typedef struct _tTask {

	/* stack保存了最后保存环境参数的地址位置，用于后续恢复 */
    tTaskStack * stack;

    /* 堆桟的起始地址 */
    uint32_t *stackBase;
    /* 堆桟大小 */
    uint32_t stackSize;

    /* 连接节点 */
    tNode linkNode;

    /* 任务延时计数器 */
    uint32_t delayTicks;

    /* 任务优先级 */
    uint32_t prio;

    /* 延时节点 */
    tNode delayNode;

    /* 当前任务状态 */
    uint32_t state;

    /* 当前剩余的时间片 */
    uint32_t slice;

    /* 挂起次数 */
    uint32_t supendCount;


    /* 任务删除时的清理函数 */
    void (*clean) (void * param);
    /* 传递给清理函数的参数 */
    void * cleanParam;
    /* 请求删除得标志 */
    uint8_t requestsDeleteFlag;


    /* 任务正在等待的事件类型 */
    struct _tEvent * waitEvent;
    /* 等待事件的消息存储位置 */
    void * waitEventMsg;
    /* 等待事件的结果 */
    uint32_t waitEventResult;


    /* 等待是件结果 */
    uint32_t waitEventGroupType;
    /* 等待事件标志 */
    uint32_t waitEventGroupFlag;
}tTask;



typedef struct  _tTaskInfo
{
    /* 任务延时计时器 */
    uint32_t delayTicks;
    /* 任务优先级 */
    uint32_t prio;
    /* 任务当前状态 */
    uint32_t state;
    /* 任务剩余时间片 */
    uint32_t slice;
    /* 任务挂起次数 */
    uint32_t supendCount;

    /* 堆桟总容量 */
    uint32_t stackSize;
    /* 堆桟剩余量 */
    uint32_t stackFree;
}tTaskInfo;

/*---------------------------------------------------------------------------------------------------------------------
 * TASK EXTERN 
 *--------------------------------------------------------------------------------------------------------------------*/

/* 当前任务：记录当前是哪个任务正在运行 */
extern tTask * currentTask;

/* 下一个将即运行的任务 */
extern tTask * nextTask;

extern tTask * idleTask;

/* 所有任务的指针数组 */
extern tList  taskPrioTable[LinOS_PRI_COUNT];

/* 调度锁计数器 */
extern uint8_t schedLockCount;

/* 延时队列 */
extern tList taskDelayList;

/* 任务优先级的标记位置结构 */
extern Bitmap_t LinOSBitmap;
/*---------------------------------------------------------------------------------------------------------------------
 * TASK CREATION API
 *--------------------------------------------------------------------------------------------------------------------*/

void vTaskInit (tTask * task, void (*entry)(void *), void *param, uint32_t prio, uint32_t * stack, uint32_t stackSize);

/* 任务调度相关函数 */
void vTaskSched(void);
void vTaskSystemTickHandler (void);



/* 调度器相关函数 */
void vTaskSchedDisable(void);
void vTaskSchedEnable(void);
void vTaskSchedInit(void);

/*----------------------------------------------------------
 * 任务队列相关 （就绪）
 *----------------------------------------------------------*/
/* 获取最高优先级任务 */
tTask *tTaskHighestReady(void);
/* 将任务加入就绪列表 */
void vTaskSchedRdy(tTask * task);
/* 将任务从就绪列表中移除 */
void vTaskSchedUnRdy(tTask * task);
/* 将任务从就绪列表中移除 */
void vTaskRdyListRemove(tTask * task);
/*----------------------------------------------------------
 * 任务队列相关 （阻塞）
 *----------------------------------------------------------*/
void vTaskDelayInit(void);
/* 将任务加入延时队列 */
void tTaskDelayWait(tTask * task, uint32_t ticks);
/* 将延时的任务从延时队列中唤醒 */
void vTaskDelayWakeUp(tTask * task);

/*----------------------------------------------------------
 * 任务队列相关 （挂起）
 *----------------------------------------------------------*/
/* 挂起任务 */
void vTaskSupend(tTask * task);
/* 将任务从挂起态唤醒 */
void vTaskResume(tTask * task);


/*----------------------------------------------------------
 * 任务队列相关 （删除）
 *----------------------------------------------------------*/
/**
 * @Author: YangSL
 * @Description: 设置任务被删除时调用的清理函数
 * @param {tTask *} task  :待设置的任务
 * @param {void} *param   :清理函数入口地址
 * @param {void *} param  :传递给清理函数的参数
 */
void vTaskSetCleanCallback(tTask * task, void (*clean )(void *param), void * param);

/**
 * @Author: YangSL
 * @Description: 强制删除任务
 * @param {tTask *} task
 */
void vTaskForceDelete(tTask * task);

/**
 * @Author: YangSL
 * @Description: 请求删除某个任务，最后由任务自己决定是否删除自己
 * @param {tTask *} task
 */
void vTaskRequestsDelete(tTask * task);

/**
 * @Author: YangSL
 * @Description: 检查是否已经被请求删除自己
 * @param {tTask *} task
 */
uint8_t uTaskCheckDelete(void);

/**
 * @Author: YangSL
 * @Description: 删除自己
 */
void vTaskDeleteSelf(void);


/*----------------------------------------------------------
 * Others
 *----------------------------------------------------------*/
/* 任务级延时函数 */
void vTaskDelay(uint32_t delay);


/**
 * @Author: YangSL
 * @Description: 获取任务相关信息
 * @param {tTask *} task   ：需要查询的任务
 * @param {tTaskInfo} info ：任务信息存储结构
 */
void vTaskGetInfo(tTask * task, tTaskInfo * info);

/**
 * @Author: YangSL
 * @Description: 获取CPU使用率
 */
float fTaskGetCpuUsage(void);
#endif
