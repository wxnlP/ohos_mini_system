#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"

/** 
 * gpio_demo/gpio_interrupt_demo.c
 * GPIO中断使用案例
 * 将GPIO输入案例的按键程序放在中断执行。
 */ 

/* LED和KEY引脚号 */
#define LED_Pin         2
#define KEY1_Pin        11
#define KEY2_Pin        12


static void Key1Callback(char* params)
{
    printf("[INFO] KEY1按键按下！\r\n");
    IoTGpioSetOutputVal(LED_Pin, IOT_GPIO_VALUE1);
}

static void Key2Callback(char* params)
{
    printf("[INFO] KEY2按键按下！\r\n");
    IoTGpioSetOutputVal(LED_Pin, IOT_GPIO_VALUE0);
}

static void InterruptTask(void* params)
{
    /* GPIO初始化 */
    IoTGpioInit(LED_Pin);
    IoTGpioInit(KEY1_Pin);
    IoTGpioInit(KEY2_Pin);
    /* 配置引脚的复用功能(海思SDK) */
    hi_io_set_func(LED_Pin, HI_IO_FUNC_GPIO_2_GPIO);
    hi_io_set_func(KEY1_Pin, HI_IO_FUNC_GPIO_11_GPIO);
    hi_io_set_func(KEY2_Pin, HI_IO_FUNC_GPIO_12_GPIO);
    /* 配置引脚的输入输出模式 */
    IoTGpioSetDir(LED_Pin, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(KEY1_Pin, IOT_GPIO_DIR_IN);
    IoTGpioSetDir(KEY2_Pin, IOT_GPIO_DIR_IN);
    /* 输入模式的引脚设置上拉电阻 */
    hi_io_set_pull(KEY1_Pin, HI_IO_PULL_UP);
    hi_io_set_pull(KEY2_Pin, HI_IO_PULL_UP);
    /* 配置中断: 
     * 1.边沿触发 2.下降沿触发 3.回调函数传入不同IoId和电平
     */
    IoTGpioRegisterIsrFunc(
        KEY1_Pin, 
        IOT_INT_TYPE_EDGE, 
        IOT_GPIO_EDGE_FALL_LEVEL_LOW, 
        Key1Callback,
        NULL
        );
    IoTGpioRegisterIsrFunc(
        KEY2_Pin, 
        IOT_INT_TYPE_EDGE, 
        IOT_GPIO_EDGE_FALL_LEVEL_LOW, 
        Key2Callback,
        NULL
        );
    while (1)
    {
        osDelay(10);
    }   
}

static void InterruptEntry(void)
{
     osThreadAttr_t attr = {
        .name = "InterruptThread",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    if (osThreadNew(InterruptTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

// SYS_RUN(InterruptEntry);