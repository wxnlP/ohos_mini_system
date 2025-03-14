#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"

/** 
 * gpio_demo/gpio_output_demo.c
 * GPIO输出案例
 * 熟悉GPIO的API、输出配置流程，让LED灯以500ms为间隔闪烁。
 */ 

/* LED引脚号 */
#define LED_Pin     2

/* LED任务 */
static void LedTask(void *params)
{
    /* 初始化GPIO */
    IoTGpioInit(LED_Pin);
    /* 设置引脚为GPIO功能(海思SDK) */
    hi_io_set_func(LED_Pin, HI_IO_FUNC_GPIO_2_GPIO);
    /* 设置引脚为输出模式 */
    IoTGpioSetDir(LED_Pin, IOT_GPIO_DIR_OUT);
    while(1) 
    {
        /* 设置引脚的高电平状态 */
        IoTGpioSetOutputVal(LED_Pin, IOT_GPIO_VALUE1);
        printf("[INFO] LED ON.\r\n");
        osDelay(50);
        /* 设置引脚的低电平状态 */
        IoTGpioSetOutputVal(LED_Pin, IOT_GPIO_VALUE0);
        printf("[INFO] LED OFF.\r\n");
        osDelay(50);
    }
}

/* LED任务入口函数 */
static void LedEntry(void)
{
    osThreadAttr_t attr = {
        .name = "LedThread",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    if (osThreadNew(LedTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

// SYS_RUN(LedEntry);