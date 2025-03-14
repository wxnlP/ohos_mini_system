#include <stdio.h>
#include <stdint.h>
#include "iot_gpio.h"
#include "iot_i2c.h"
#include "iot_errno.h"
#include "hi_io.h"
#include "oled_fonts.h"
#include "oled.h"


// 从机地址(最低位补零后)
#define OLED_SLAVE_ADDR     0x78
// I2C设备ID，即使用I2C1
#define OLED_I2C1_ID        1
// I2C1引脚
#define OLED_I2C1_SDA       0
#define OLED_I2C1_SCL       1
// I2C1的波特率，即传输速率 400kHZ
#define OLED_I2C1_BAUDRATE  (400 * 1000)
// 控制字节-写命令
#define OLED_CTR_CMD        0x00
// 控制字节-写数据
#define OLED_CTR_DATA       0x40
// 计算数组长度
#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

/* 存储发送和接收的数据 */
typedef struct
{
    // 要发送的数据的指针
    unsigned char *sendBuf;
    // 要发送的数据长度
    unsigned int sendLen;
    // 要接收的数据的指针
    unsigned char *receiveBuf;
    // 要接收的数据长度
    unsigned int receiveLen;
} IotI2cData;


/**
 * @brief 指定内存地址写入一个字节命令/数据
 * @param memAddr 内存地址，可以选择写入命令 0x00 或写入数据 0x40 模式
 * @param byte 需要写入的命令/数据
 * @retval 成功返回 IOT_SUCCESS，失败返回 IOT_FAILURE
 */
static uint32_t I2cMemWriteByte(uint8_t memAddr, uint8_t byte)
{
    /* 定义发送的 */
    uint8_t buffer[] = {memAddr, byte};
    IotI2cData i2cData = {0};
    /* 用结构体处理发送数据 */
    i2cData.sendBuf = buffer;
    i2cData.sendLen = sizeof(buffer) / sizeof(buffer[0]);
    return IoTI2cWrite(OLED_I2C1_ID, OLED_SLAVE_ADDR, i2cData.sendBuf, i2cData.sendLen);
}

/**
 * @brief 写一个字节命令
 * @param cmd 需写入的命令
 * @retval 成功返回 IOT_SUCCESS，失败返回 IOT_FAILURE
 */
static uint32_t WriteCmd(uint8_t cmd)
{
    return I2cMemWriteByte(OLED_CTR_CMD, cmd);
}

/**
 * @brief 写一个字节数据
 * @param data 需写入的数据
 * @retval 成功返回 IOT_SUCCESS，失败返回 IOT_FAILURE
 */
static uint32_t WriteData(uint8_t data)
{
    return I2cMemWriteByte(OLED_CTR_DATA, data);
}

/**
 * @brief 初始化OLED
 */
uint32_t OledInit(void)
{
    static const uint8_t initCmds[] = {
        0xAE, // 显示关闭
        0x00, // 页寻址模式时，设置列地址的低4位为0000
        0x10, // 页寻址模式时，设置列地址的高4位为0000
        0x40, // 设置起始行地址为第0行
        0xB0, // 页寻址模式时，设置页面起始地址为PAGE0
        0x81, // 设置对比度
        0xFF, // 对比度数值
        0xA1, // set segment remap
        0xA6, // 设置正常显示。0对应像素熄灭，1对应像素亮起
        0xA8, // --set multiplex ratio(1 to 64)
        0x3F, // --1/32 duty
        0xC8, // Com scan direction
        0xD3, // -set display offset
        0x00, //
        0xD5, // set osc division
        0x80, //
        0xD8, // set area color mode off
        0x05, //
        0xD9, // Set Pre-Charge Period
        0xF1, //
        0xDA, // set com pin configuartion
        0x12, //
        0xDB, // set Vcomh
        0x30, //
        0x8D, // set charge pump enable
        0x14, //
        0xAF, // 显示开启
    };
    /* 初始化I2C的GPIO引脚 */
    IoTGpioInit(OLED_I2C1_SDA);
    hi_io_set_func(OLED_I2C1_SDA, HI_IO_FUNC_GPIO_0_I2C1_SDA);
    IoTGpioInit(OLED_I2C1_SCL);
    hi_io_set_func(OLED_I2C1_SCL, HI_IO_FUNC_GPIO_1_I2C1_SCL);
    /* 初始化I2C1的波特率 */
    IoTI2cInit(OLED_I2C1_ID, OLED_I2C1_BAUDRATE);
    /* 发送初始化命令 */ 
    for (int i=0 ; i < ARRAY_SIZE(initCmds) ; i++) {
        uint32_t status = WriteCmd(initCmds[i]);
        if (status != IOT_SUCCESS) {
            return status;
        }
    }
    return IOT_SUCCESS;
}

/** 
 * @brief 设置显示的起始位置
 * @param x: 横坐标，取值范围是0~127，对应128个像素点，单位为1bit。
 * @param y: 纵坐标，取值范围是0~7，对应8个pages，单位是字节(8bits)。
 */
void OledSetPosition(uint8_t x, uint8_t y)
{
    /* 起始页(y坐标) */
    WriteCmd(0xB0 + y);
    /* 列地址低四位 */
    WriteCmd(0x0F & x);
    /* 列地址高四位 */
    WriteCmd(0x10 | ((0xF0 & x) >> 4));
}

/**
 * @brief 全屏填充，即按128列数据一致的方式填充
 * @param data 填充的1字节数据
 */
void OledFillScreen(uint8_t data)
{
    uint8_t m=0;
    uint8_t n=0;
    /* 遍历8个page，一个page有8像素点 */
    for (m=0 ; m < 8 ; m++) {
        // 设置page地址：0~7
        WriteCmd(0xB0 + m);
        // 设置显示位置为第0列
        WriteCmd(0x00);
        WriteCmd(0x10);
        // 遍历128列
        for (n=0 ; n < 128 ; n++) {
            WriteData(data);
        }
    }
}

/**
 * @brief 显示一个字符
 * @param x: x坐标，1像素为单位 
 * @param y: y坐标，8像素为单位 
 * @param ch: 要显示的字符
 * @param font: 字库
 */
void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font)
{
    /* 字库数组下标 */
    uint8_t c;
    /* 循环计数 */
    uint8_t i = 0;
    /* 空格的ASCII码十进制数为32,(字符-32)即为其在字库数组的下标 */
    c = ch - ' ';
    /* 区分字体大小, FONT8x16需分两个pages */
    if (font == FONT8x16) {
        // 定位
        OledSetPosition(x, y);
        // 写第一个page
        for (i=0 ; i < 8 ; i++) {
            WriteData(F8X16[c*16 + i]);
        }
        // 重新定位
        OledSetPosition(x, y + 1);
        // 写第二个page
        for (i=0 ; i < 8 ; i++) {
            WriteData(F8X16[c * 16 + i + 8]);
        }
    }
    /* FONT6x8,一个page足矣 */ 
    else {
        // 定位
        OledSetPosition(x, y);
        // 写一个page
        for (i=0 ; i < 6 ; i++) {
            WriteData(F6x8[c][i]);
        }
    }
}

/**
 * @brief 显示字符串
 * 
 */
void OledShowString(uint8_t x, uint8_t y,  const char *str, Font font)
{
    /* 字符数组（字符串）下标 */ 
    uint8_t j = 0;
    if (str == NULL)
    {
        printf("[ERROR] param is NULL,Please check!!!\r\n");
        return;
    }
     while (str[j])
    {
        // 显示一个字符
        OledShowChar(x, y, str[j], font);
        // 设置字符间距
        x += 8;
        // 如果下一个要显示的字符超出了OLED显示的范围，则换行
        if (x > 120) {
            x = 0;
            y += 2;
        }
        // 下一个字符
        j++;
    }
}