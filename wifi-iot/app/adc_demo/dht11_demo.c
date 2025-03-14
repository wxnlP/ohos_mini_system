#include "dht11.h"
#include "cmsis_os2.h"

#define dht11_pin 4

static void Dht11Task(void* params)
{
    osDelay(100);
    uint8_t humidity_int, humidity_dec, temperature_int, temperature_dec;
    while(1) {
        printf("[INFO] Cycle Start.\r\n");
        if (Dht11ReadData(dht11_pin, &humidity_int, &humidity_dec, &temperature_int, &temperature_dec)) {
            printf("[INFO] Data Get.\r\n");
            // 将整数位和小数位整合成完整的湿度值
            float humidity = (float)humidity_int + (float)humidity_dec / 10.0;
            // 将整数位和小数位整合成完整的温度值
            float temperature = (float)temperature_int + (float)temperature_dec / 10.0;
            // 打印完整的温湿度值
            printf("[INFO] Humidity: %.1f%%\r\n", humidity);
            printf("[INFO] Temperature: %.1f°C\r\n", temperature);
        } else {
            printf("[ERROR] TIMEOUT\r\n");
        }
        osDelay(150);
    }
}

static void Dht11Entry(void)
{
     osThreadAttr_t attr = {
        .name = "Dht11Thread",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    if (osThreadNew(Dht11Task, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    } else {
        printf("[INFO] Thread Create Success.\r\n");
    }
}

// SYS_RUN(Dht11Entry);