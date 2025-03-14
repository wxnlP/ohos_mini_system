#include <stdio.h>
#include <string.h>
#include "ohos_init.h"
#include "iot_gpio.h"
#include "iot_uart.h"
#include "iot_errno.h"
#include "hi_io.h"
#include "cmsis_os2.h"

/**
 * uart_demo/uart_demo.c
 * 串口接收信息，并将接收的信息发送回去
 */

#define UART_ID        1
#define UART_TX_PIN    6
#define UART_RX_PIN    5

static void Uart1Init(void)
{
    /* 复用UART1到GPIO5、GPIO6 */
    IoTGpioInit(UART_TX_PIN);
    IoTGpioInit(UART_RX_PIN);
    hi_io_set_func(UART_TX_PIN, HI_IO_FUNC_GPIO_6_UART1_TXD);
    hi_io_set_func(UART_RX_PIN, HI_IO_FUNC_GPIO_5_UART1_RXD);
    /* 配置UART1的属性 */
    IotUartAttribute uartAttr = {
        // 波特率
        .baudRate = 9600,
        // 数据位长度
        .dataBits = IOT_UART_DATA_BIT_8,
        // 停止位长度
        .stopBits = IOT_UART_STOP_BIT_1,
        // 奇偶校验
        .parity = IOT_UART_PARITY_NONE,
        // 发送且接收
        .rxBlock = IOT_UART_BLOCK_STATE_BLOCK,
        .txBlock = IOT_UART_BLOCK_STATE_BLOCK,
        .pad = 0,
    };
    if (IoTUartInit(UART_ID, &uartAttr) != IOT_SUCCESS) {
        printf("[ERROR] UART INIT ERR.\r\n");
    }
}

static void UartTask(void* arg)
{
    /* 初始化串口 */
    Uart1Init();
    int value = 1;
    unsigned char buffer[64] = {0};
    snprintf(buffer, sizeof(buffer),"[INFO] AT+LOCK=%d\r\n", value);
    while (1) 
    {
        if (IoTUartWrite(UART_ID, buffer, strlen(buffer)) < 0) {
            printf("[ERROR] IoTUartWrite ERR.\r\n");
        } else {
            printf("[INFO] IoTUartWrite RTT.\r\n");
        }
        osDelay(100);
    }
}

/* 入口函数 */
static void UartEntry(void)
{
     osThreadAttr_t attr = {
        .name = "UartTask",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    if (osThreadNew(UartTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    } else {
        printf("[INFO] Thread Create RT.\r\n");
    }
}

// SYS_RUN(UartEntry);