#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_pwm.h"
#include "hi_io.h"
#include "hi_pwm.h"

/** 
 * gpio_demo/gpio_pwm_demo.c
 * PWM使用案例
 * 熟悉PWM的API，实现呼吸灯。
 */ 

#define LED_Pin         2

static void PwmLedTask(void* params)
{
    int i;
    /* GPIO初始化 */
    IoTGpioInit(LED_Pin);
    /* 配置引脚的复用功能为PWM(海思SDK) */
    hi_io_set_func(LED_Pin, HI_IO_FUNC_GPIO_2_PWM2_OUT);
    /* 配置引脚的输入输出模式 */
    IoTGpioSetDir(LED_Pin, IOT_GPIO_DIR_OUT);
    /* PWM初始化 */
    // IoTPwmInit(LED_Pin);
    hi_pwm_init(LED_Pin);
    while(1)
    {
        /* 配置PWM参数，占空比递增后递减、频率4000HZ */
        for (i=0 ; i<40000 ; i+=200) {
            // IoTPwmStart(LED_Pin, i, 4000);
            /* 4000 = 160,000,000/40000 */
            hi_pwm_start(LED_Pin, i, 40000);
            osDelay(1);
        }
        for (i=0 ; i<40000 ; i+=200) {
            // IoTPwmStart(LED_Pin, 100-i, 4000);
            hi_pwm_start(LED_Pin, 40000-i, 40000);
            osDelay(1);
        }
        // osDelay(2);
    }
}

static void PwmEntry(void)
{
     osThreadAttr_t attr = {
        .name = "PwmThread",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    if (osThreadNew(PwmLedTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

SYS_RUN(PwmEntry);