#include "dht11.h"

/* 微秒级延时 */
static void Dht11DelayUs(unsigned int us)
{
    hi_udelay(us);
}

/* 毫秒级延时 */
static void Dht11DelayMs(unsigned int ms)
{
    hi_udelay(ms*1000);
}

/* 输入模式初始化 */
static void Dht11InputInit(unsigned int dht_data_pin)
{
    // IoTGpioInit(dht_data_pin);
    hi_io_set_func(dht_data_pin, HI_IO_FUNC_GPIO_4_GPIO);
    IoTGpioSetDir(dht_data_pin, IOT_GPIO_DIR_IN);
    hi_io_set_pull(dht_data_pin, HI_IO_PULL_UP);
}

/* 输出模式初始化 */
static void Dht11OutputInit(unsigned int dht_data_pin)
{
    IoTGpioInit(dht_data_pin);
    hi_io_set_func(dht_data_pin, HI_IO_FUNC_GPIO_4_GPIO);
    IoTGpioSetDir(dht_data_pin, IOT_GPIO_DIR_OUT);
}

/* 起始条件 */
static void Dht11Start(unsigned int dht_data_pin)
{
    /* 输出模式，拉低至少18ms，拉高20~40us */
    Dht11OutputInit(dht_data_pin);
    IoTGpioSetOutputVal(dht_data_pin, IOT_GPIO_VALUE0);
    Dht11DelayMs(20);
    IoTGpioSetOutputVal(dht_data_pin, IOT_GPIO_VALUE1);
    Dht11DelayUs(30);
    /* 切换为输入模式 */
    Dht11InputInit(dht_data_pin);
}

/* 等待指定的电平 */
static uint8_t Dht11WaitForLevel(unsigned int dht_data_pin, uint8_t level, unsigned int timeout_us)
{
    unsigned int elapsed = 0;
    IotGpioValue val;
    /* 超时判断 */
    while (elapsed < timeout_us) {
        IoTGpioGetInputVal(dht_data_pin, &val);
        if (val == level) {
            return 1;
        }
        Dht11DelayUs(1);
        elapsed++;
    }
    return 0;
}

/* 读取一个字节数据 */
static uint8_t Dht11ReadByte(unsigned int dht_data_pin)
{
    // 位数据“0”的格式为：54微秒的低电平和23-27微秒的高电平，
    // 位数据“1”的格式为：54微秒的低电平加68-74微秒的高电平
    uint8_t i, byte = 0x00;
    IotGpioValue value;
    for (i=0 ; i<8 ; i++) {
        /* 等待低电平结束，以便通过高电平的持续时间判断 0/1 */
        if (Dht11WaitForLevel(dht_data_pin, 1, 100) == 0) {
            printf("[WaitForLevel ERROR] Timeout-2.1\r\n");
            return 0;
        }
        /* 延迟40us */
        Dht11DelayUs(40);
        /* 若此时引脚为高电平则数据位为 1，否则为 0 */
        IoTGpioGetInputVal(dht_data_pin, &value);
        if (value == IOT_GPIO_VALUE1) {
            /* 高位先行，置第i位为 1 */
            byte |= (0x80 >> i);
            /* 等待高电平结束 */
            if (Dht11WaitForLevel(dht_data_pin, 0, 100) == 0) {
                printf("[WaitForLevel ERROR-2.2] Timeout\r\n");
                return 0;
            }
        }
    }
    return byte;
}

/* 读取40Bit */
uint8_t Dht11ReadData(unsigned int dht_data_pin, uint8_t* humidity_int, uint8_t* humidity_dec, 
    uint8_t* temperature_int, uint8_t* temperature_dec)
{
    uint8_t rh_int, rh_dec, tp_int, tp_dec, checksum;
    /* 起始条件 */
    Dht11Start(dht_data_pin);
    /* 等待DHT11设备响应低电平 */
    if (Dht11WaitForLevel(dht_data_pin, 0, 100) == 0) {
        printf("[WaitForLevel ERROR] Timeout-1.1\r\n");
        return 0;
    }
    /* 等待DHT11设备响应高电平 */   
    if (Dht11WaitForLevel(dht_data_pin, 1, 100) == 0) {
        printf("[WaitForLevel ERROR] Timeout-1.2\r\n");
        return 0;
    }
    /* 分别读取40Bit数据 */
    // 温湿度数据
    rh_int = Dht11ReadByte(dht_data_pin);
    rh_dec = Dht11ReadByte(dht_data_pin);
    tp_int = Dht11ReadByte(dht_data_pin);
    tp_dec = Dht11ReadByte(dht_data_pin);
    // 校验位
    checksum = Dht11ReadByte(dht_data_pin);
    // 验证校验位
    
    if (checksum == (rh_int + rh_dec + tp_int + tp_dec)) {
        printf("[INFO] Checksum PASS\r\n");
        *humidity_int = rh_int;
        *humidity_dec = rh_dec;
        *temperature_int = tp_int;
        *temperature_dec = tp_dec;
        return 1;
    } else {
        printf("[ERROR] Data Error\r\n");
        return 0;
    }
}
