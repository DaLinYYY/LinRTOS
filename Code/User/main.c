/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-06
 * @LastEditTime: 2021-08-09 22:51:27
 * @Description: 
 */
#include "LinOS.h"
#include "port.h"


void delay (int count)
{
    while (--count > 0);
}

/* 任务1和任务2的任务结构，以及用于堆栈空间 */
tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;
tTaskStack task1Env[LinOS_STACK_SIZE];
tTaskStack task2Env[LinOS_STACK_SIZE];
tTaskStack task3Env[LinOS_STACK_SIZE];
tTaskStack task4Env[LinOS_STACK_SIZE];

float ans;
int task1Flag;
void task1 (void * param)
{	
    

    for (;;)
    {         
        ans = fTaskGetCpuUsage();
        task1Flag = 1;
        vTaskDelay(5);
        task1Flag = 0;
		vTaskDelay(5);
    }

}

int task2Flag;
void task2 (void * param)
{
//    uMutexTake(&mutex1, 0);
    for (;;)
    {
        task2Flag = 1;
        vTaskDelay(1);
        task2Flag = 0;
		vTaskDelay(1);
    }
}

int task3Flag;
void task3 (void * param)
{
    for (;;)
    {          
        task3Flag = 1;
        vTaskDelay(1);
        task3Flag = 0;
		vTaskDelay(1);
    }
}

int task4Flag;
void task4 (void * param)
{
    for (;;)
    {

        task4Flag = 1;
        vTaskDelay(1);
        task4Flag = 0;
		vTaskDelay(1);

    }
}



int main ()
{
    vTaskSchedInit();
	vTaskSetTickPeriod(LinOS_SYSTICK_MS);
    /* 初始化延时队列 */
    vTaskDelayInit();


    // 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
    vTaskInit(&tTask1, task1, (void *)0x11111111, 0, task1Env, sizeof(task1Env));
    vTaskInit(&tTask2, task2, (void *)0x22222222, 1, task2Env, sizeof(task2Env));
    vTaskInit(&tTask3, task3, (void *)0x33333333, 1, task3Env, sizeof(task3Env));
    vTaskInit(&tTask4, task4, (void *)0x44444444, 1, task4Env, sizeof(task4Env));
	

    /* 设置启动任务 */
    nextTask = tTaskHighestReady();

    vTaskSetTickPeriod(1);
    /* 运行第一个任务 */
    taskFristRun();
    
    return 0;
}
