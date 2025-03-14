#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

typedef struct {
    int value;
    char str[16];
}Data;

/* 线程要运行的函数 */ 
void Rtosv2Thread1(void* params) 
{
    /* 接收线程参数 */
    Data* data_;
    data_ = params;
    /* 系统级延时函数，延时10个Tick */
    osDelay(100);
    /* 获取当前线程的ID */
    osThreadId_t thread1Id = osThreadGetId();
    printf("[INFO]Thread ID: %p\r\n", thread1Id);
    /* 处理参数 */
    printf("[INFO]%d    %f\r\n", data_->value, data_->str);
    /* 主程序 */
    static int count = 0;
    while (1) {
        count++;
        printf("[INFO]Thread1 count: %d\r\n", count);
        osDelay(10);
    }
}

void Rtosv2Thread2(void* params) 
{
    osThreadId_t thread2Id = osThreadGetId();
    printf("[INFO]Thread ID: %p\r\n", thread2Id);
    while (1) {
        printf("[INFO]Thread2\r\n");
        osDelay(20);
    }
}

/* 创建任务线程 */
void ThreadTestTask(void) 
{
    /* 定义线程的属性结构体 */
    osThreadAttr_t attr_1 = {
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
    osThreadAttr_t attr_2 = {
        .name = "Thread2",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    /* 定义线程参数 */
    Data data = { 12, "OpenHarmony" };
    /* 创建线程 */
    osThreadNew(Rtosv2Thread1, &data, &attr_1);
    osThreadNew(Rtosv2Thread2, NULL, &attr_2);
}

// APP_FEATURE_INIT(ThreadTestTask);