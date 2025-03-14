#include <stdio.h>
#include <stdint.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "oled.h"

/**
 * oled_demo/oled_demo.c 
 * OLED案例分析
 * 使用自己编写的各个OLED驱动显示字符
 */

static void OledTask(void* params)
{
    /* 初始化 */
    OledInit();
    OledFillScreen(0x00);
    /* FONT6x8 */
    OledShowChar(0, 0, 'A', FONT6x8);
    OledShowChar(0, 1, 'B', FONT6x8);
    OledShowChar(0, 2, 'C', FONT6x8);
    OledShowChar(0, 3, 'D', FONT6x8);
    OledShowChar(0, 4, 'E', FONT6x8);
    OledShowChar(0, 5, 'F', FONT6x8);
    OledShowChar(0, 6, 'G', FONT6x8);
    OledShowChar(0, 7, 'H', FONT6x8);
    /* FONT8x16 */
    OledShowChar(8, 0, 'I', FONT8x16);
    OledShowChar(8, 2, 'J', FONT8x16);
    OledShowChar(8, 4, 'K', FONT8x16);
    OledShowChar(8, 6, 'L', FONT8x16);
    /* 字符串 */
    OledShowString(16, 0, "OpenHarmony", FONT8x16);
    while(1)
    {
        osDelay(10);
    } 
}

static void OledEntry(void)
{
     osThreadAttr_t attr = {
        .name = "OledThread",
        .stack_size = 1024,
        .priority = osPriorityNormal
    };
    if (osThreadNew(OledTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    } else {
        printf("[INFO] Thread Create Success.\r\n");
    }
}

// SYS_RUN(OledEntry);