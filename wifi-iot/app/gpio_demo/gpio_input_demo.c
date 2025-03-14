#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"

/** 
 * gpio_demo/gpio_input_demo.c
 * GPIO输入案例
 * 熟悉GPIO的API、输入配置流程(是否上拉或下拉电阻)，通过按键控制LED点亮与熄灭。
 */ 

/* LED和KEY引脚号 */
#define LED_Pin         2
#define KEY1_Pin        11
#define KEY2_Pin        12

/* 按键任务 */
static void ButtonTask(void* params)
{
    /* 局部变量 */
    IotGpioValue key1_value, key2_value;
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
    while (1)
    {
        /* 获取输入电平 */
        IoTGpioGetInputVal(KEY1_Pin, &key1_value);
        IoTGpioGetInputVal(KEY2_Pin, &key2_value);
        if (key1_value == 0) {
            osDelay(2);
            printf("[INFO] KEY1的电平: %d\r\n", key1_value);
            /* 点亮LED */
            IoTGpioSetOutputVal(LED_Pin, IOT_GPIO_VALUE1);
        }
        if (key2_value == 0) {
            osDelay(2);
            printf("[INFO] KEY2的电平: %d\r\n", key2_value);
            /* 熄灭LED */
            IoTGpioSetOutputVal(LED_Pin, IOT_GPIO_VALUE0);
        }
        osDelay(20);
    }
}

static void ButtonEntry(void)
{
     osThreadAttr_t attr = {
        .name = "ButtonThread",
        .stack_size = 4096,
        .priority = osPriorityNormal
    };
    if (osThreadNew(ButtonTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

// SYS_RUN(ButtonEntry);