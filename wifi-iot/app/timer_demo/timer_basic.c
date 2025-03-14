#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

/**timer_basic.c
 * 软件定时器案例分析
 * 创建定时器，每 1s 调用一次回调函数使得计数值加一，主线程中等待 5s 删除定时器。
 */
osTimerId_t g_timerID;
static uint8_t g_cnt = 0;

/* 回调函数 */
void time_out_callback(void* params)
{
    g_cnt++;
    printf("[Callback] Timeout! ->%d\r\n", g_cnt);
}

/* 主线程函数 */
void MainThread(void* params)
{
    /* 定时器属性参数 */
    osTimerAttr_t attr_timer = {
        .name = "SoftTimer",
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0
    };
    /* 创建定时器 */ 
    g_timerID = osTimerNew(time_out_callback, osTimerPeriodic, NULL, &attr_timer);
    /* 判断g_timerID，定时器是否创建成功 */
    if (g_timerID != NULL) {
        printf("[Timer Create] 定时器创建成功！\r\n");
    } else {
        printf("[Timer Create] 定时器创建失败！\r\n");
    }
    /* 打开定时器，定时1s */
    osTimerStart(g_timerID, 100);
    /* 阻塞3s */
    osDelay(500);
    while(1) {
        /* 若定时器存在，停止并删除定时器 */
        if (g_timerID != NULL) {
            osTimerStop(g_timerID);
            osTimerDelete(g_timerID);
            printf("[Timer Delete] 定时器删除成功！\r\n");
            /* 自杀线程 */
            osThreadExit();
            g_timerID = NULL;
        }
        osDelay(10);
    }
    
}

/* 创建任务线程 */
void TimerTest(void) 
{
    /* 定义线程的属性结构体 */
    osThreadAttr_t attr_timer = {
        .name = "Thread1",              // 为线程命名（必须）
        .attr_bits = 0,                 // 一般先设为 0，有特殊属性需求时再按位设置
        .cb_mem = NULL,                 // 若不手动分配控制块内存，可设为 NULL
        .cb_size = 0,                   // 控制块内存大小，配合 cb_mem使用，设为 0表示使用默认方式
        .stack_mem = NULL,              // 若不手动分配栈内存，可设为 NULL
        .stack_size = 1024,             // 为线程分配栈大小，这里设为 1024字节，需根据实际需求调整（必须）
        .priority = osPriorityNormal,   // 设置线程优先级为正常优先级（必须）
        .tz_module = 0,                 // TrustZone模块标识符，若无特殊需求设为 0
        .reserved = 0                   // 保留字段设为 0
    };
    /* 创建线程 */
    osThreadNew(MainThread, NULL, &attr_timer);
}

// APP_FEATURE_INIT(TimerTest);