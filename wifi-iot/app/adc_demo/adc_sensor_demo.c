#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_adc.h"


/**
 * adc_demo/adc_demo.c
 * ADC读取模拟值案例
 * HAL接口缺失ADC的API，故本节API均采用海思SDK接口。
 * 利用ADC通道读取红外传感器的模拟值，信号引脚接传感器的AO(Analog)。
 * 同时一些宏和函数参数，我们尽量使用API定义好的枚举或结构体，因此要学会阅读源码注释。
 */

#define SensorAdcChannel  HI_ADC_CHANNEL_4

static void SensorAdcTask(void* params)
{
    /* 定义ADC读取的模拟值的存储地址 */
    uint16_t value;
    while(1) 
    {
        if (hi_adc_read(SensorAdcChannel, &value, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) == HI_ERR_SUCCESS) {
            printf("[INFO]ADC_VALUE = %d\r\n", value);
        }
        osDelay(100);
    }

}

static void AdcEntry(void)
{
     osThreadAttr_t attr = {
        .name = "AdcThread",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    if (osThreadNew(SensorAdcTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

// SYS_RUN(AdcEntry);