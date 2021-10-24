/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-06 
 * @LastEditTime: 2021-08-10 11:31:42
 * @Description: 
 */

#ifndef LINOS_H
#define LINOS_H


// 标准头文件，里面包含了常用的类型定义，如uint32_t
#include <stdint.h>
#include <string.h>
#include "LinOSConfig.h"

#include "Lists.h"
#include "Tasks.h"
#include "Sem.h"
#include "Mailbox.h"
#include "Mempool.h"
#include "EventGroup.h"
#include "Mutex.h"
#include "Timer.h"

#include "port.h"

/* LinOS错误码 */
typedef enum _eError
{
    eErrorNoError = 0,                  // 没有错误
    eErrorTimeout,                      // 超时
    eErrorResourceUnavaliable,          // 资源不可用
    eErrorDelete,                       // 资源被删除
    eErrorResourceFull,                 // 邮箱满了
    eErrorOwner,                        // 任务不匹配
}eError;




#endif /* LINOS_H */
