#include <stdio.h> 
#include "ohos_init.h"
#include "cmsis_os2.h"

/** 
 * mutex_demo.c
 * 互斥锁案例
 * 多线程访问同一个变量，用互斥锁实现安全访问。同时实现多线程使用同一个任务函数。
 */

/* 多个线程需要访问的全局变量 */
static int g_param = 0;

/* 任务函数 */
void CountThread(void* params)
{
    /* 接收线程参数 */
    osMutexId_t *mid = params;
    /* 线程主循环 */
    while(1) 
    {
        /* 获取互斥锁A，超时时间为100ms，若成功返回"osOK" */
        if(osMutexAcquire(*mid, 100) == osOK) {
            g_param++;
            /* g_param为偶数 */
            if (g_param % 2 == 0) {
                printf("[g_param] 偶数-> %d\r\n", g_param);
            } 
            /* g_param为奇数 */
            else {
                printf("[g_param] 奇数-> %d\r\n", g_param);
            }
            printf("[Info] 任务处理完成！\r\n");
        }
        /* 释放互斥锁A */
        osMutexRelease(*mid);
        /* 完成一次任务阻塞50ms，给其他同优先级的任务执行的时间 */
        osDelay(50);
    }

}

/**
 * @brief 创建线程，封装成一个函数，便于调用
 * @param name：线程名称
 * @param func：线程函数
 * @param arg：线程函数的参数
 * @return 返回线程ID
 */
osThreadId_t newThread(char *name_, osThreadFunc_t func, void *arg)
{
    osThreadAttr_t attr = {
        .name = name_,
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    if (tid == NULL) {
        printf("[Thread Create] osThreadNew(%s) failed.\r\n", name_);
    } else{
        printf("[Thread Create] osThreadNew(%s) success, thread id: %d.\r\n", name_, tid);
    }
    return tid;
}

/* 主函数 */
void Main_Mutex(void* params)
{
    /* 定义互斥锁 */
    osMutexId_t g_MutexId_A;
    /* 互斥锁属性 */
    osMutexAttr_t attr_mutex = {
        .name = "Mutex_A",
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0
    };
    /* 创建互斥锁 */
    g_MutexId_A = osMutexNew(&attr_mutex);
    /* 判断互斥锁是否创建成功 */
    if (g_MutexId_A != NULL) {
        printf("[Mutex Create] Ok!\r\n");
    } else {
        printf("[Mutex Create] Error!\r\n");
    }
    /* 创建三个线程 */
    osThreadId_t tid1 = newThread("Thread_1", CountThread, &g_MutexId_A);
    osThreadId_t tid2 = newThread("Thread_2", CountThread, &g_MutexId_A);
    osThreadId_t tid3 = newThread("Thread_3", CountThread, &g_MutexId_A);
    osDelay(10);
    /* 获得当前占用互斥锁的线程ID */
    osThreadId_t tid = osMutexGetOwner(g_MutexId_A);
    printf("[Mutex Owner] osMutexGetOwner, thread id: %p, thread name: %s.\r\n", tid, osThreadGetName(tid));
    osDelay(100);
    /* 终止线程 */
    osThreadTerminate(tid1);
    osThreadTerminate(tid2);
    osThreadTerminate(tid3);
    /* 删除互斥锁A */
    osMutexDelete(g_MutexId_A);
}   

/* 入口函数 */ 
static void MutexTestTask(void)
{
    /* 线程属性 */ 
    osThreadAttr_t attr = {
        .name = "MutexTestTask",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    /* 创建一个线程，并将其加入活跃线程组中 */ 
    if (osThreadNew((osThreadFunc_t)Main_Mutex, NULL, &attr) == NULL)
    {
        printf("[MutexTestTask] Falied to create Main_Mutex!\n");
    }
}

// APP_FEATURE_INIT(MutexTestTask);
