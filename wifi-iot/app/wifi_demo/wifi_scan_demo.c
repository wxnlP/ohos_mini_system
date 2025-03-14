#include <stdio.h>
#include <stdint.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
// HAL接口API
#include "wifi_device.h"

/**
 * wifi_demo/wifi_scan_demo.c
 * 扫描Wi-Fi热点案例
 * -------------------------------------------------------------------------------
 * 第1步，使用`RegisterWifiEvent`接口注册Wi-Fi事件监听器。
 * 第2步，使用`EnableWifi`接口开启 Wi-Fi 设备的 STA 模式。
 * 第3步，使用`Scan`接口开始扫描 Wi-Fi 热点。
 * 第4步，在扫描状态变化事件（`OnWifiScanStateChanged`）的回调函数中监测扫描是否完成。
 * 第5步，等待扫描完成。
 * 第6步，使用`GetScanInfoList`接口获取扫描结果。
 * 第7步，显示扫描结果。
 * 第8步，使用`DisableWifi`接口关闭 Wi-Fi设 备的 STA 模式。
 */

/* 全局变量 */
// 用于表示热点扫描是否找到可用热点
int g_scanDone;

/**
 * @brief 封装wifi_device_config.h文件中的WifiSecurityType数据类型
 * 
 * @param param WifiSecurityType
 * @return 字符串类型的 WifiSecurityType
 */
static char* WifiSecurityTypeName(WifiSecurityType type)
{
    switch (type)
    {
        case WIFI_SEC_TYPE_OPEN:
            return "OPEN";
        case WIFI_SEC_TYPE_WEP:
            return "WEP";
        case WIFI_SEC_TYPE_PSK:
            return "PSK";
        case WIFI_SEC_TYPE_SAE:
            return "SAE";
        default:
            break;
    }
    return "INVALID";
}

/**
 * @brief 连接状态变化回调函数，接收的参数均声明在 wifi_linked_info.h
 * 
 * @param state 连接状态(WifiConnState)，WIFI_CONNECTED表示连接成功，WIFI_DISCONNECTED表示连接失败
 * @param info 连接信息(WifiLinkedInfo)，有多个信息参数
 * @return 无 
 */
static void WifiConnStateCallback(int state, WifiLinkedInfo *info)
{
    (void)state;    // 忽略参数state
    (void)info;     // 忽略参数info

    // 简单输出日志信息，表明函数被执行了
    printf("[CALLBACK] %s %d\r\n", __FUNCTION__, __LINE__);
}

/**
 * @brief 扫描状态变化回调函数
 * 
 * @param state 
 * @param size 
 */
static void WifiScanStateCallback(int state, int size)
{
    // 输出日志
    printf("[CALLBACK] %s %d, state = %X, size = %d\r\n", __FUNCTION__, __LINE__, state, size);
    // 扫描完成，并且找到了热点
    if (state == WIFI_STATE_AVALIABLE && size > 0)
    {
        // 不能直接调用GetScanInfoList函数，否则会有运行时异常报错
        // 可以更新全局状态变量，在另外一个线程中轮询状态变量，这种方式实现起来比较简单
        // 但需要保证更新和查询操作的原子性，逻辑才是严格正确的
        // 或者使用信号量进行通知，这种方式更好一些，更优雅
        g_scanDone = 1;
    }
}

/**
 * @brief 打印扫描结果
 * 
 */
void WifiPrintScanResult(void) 
{
    // 创建一个WifiScanInfo数组，用于存放扫描结果 
    // WifiScanInfo 结构体定义在 wifi_scan_info.h 
    // WIFI_SCAN_HOTSPOT_LIMIT 声明在 wifi_scan_info.h 
    WifiScanInfo wifiScanInfo[WIFI_SCAN_HOTSPOT_LIMIT] = {0};
    // WifiScanInfo数组大小
    uint32_t wifiScanInfoSize = WIFI_SCAN_HOTSPOT_LIMIT;
    // 获取扫描结果
    WifiErrorCode errCode = GetScanInfoList(wifiScanInfo, &wifiScanInfoSize);
    // 判断errorCode
    if (errCode != WIFI_SUCCESS) {
        printf("[ERROR] GetScanInfoList failed -> %d\r\n", errCode);
        return;
    }
    // 打印扫描结果
    for (int i=0 ; i < wifiScanInfoSize ; i++) {
        // 创建缓存区，存储 MAC 地址字符串
        static char macAddress[32] = {0};
        // 取出第 i 个扫描结果(结构体)
        WifiScanInfo info = wifiScanInfo[i];
        // 获取bssid，即热点的 MAC 地址
        unsigned char *mac = info.bssid;
        // 格式化字符串，info.bssid是一个数组，大小为 WIFI_MAC_LEN=6
        snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", 
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        // 输出结果
        printf("[INFO] AP[%d]: %s, %s, %4s, %d, %d, %d\r\n", 
            i,
            info.ssid,
            macAddress,
            WifiSecurityTypeName(info.securityType),
            info.rssi,
            info.band,
            info.frequency);
    }
}

static void WifiScanTask(void* param)
{
    // 返回值
    WifiErrorCode errCode;
    /* 绑定回调函数 */
    WifiEvent eventListener = {
        // 在连接状态发生变化时，调用WifiConnStateCallback回调函数
        .OnWifiConnectionChanged = WifiConnStateCallback,
        // 在扫描状态发生变化时，调用WifiScanStateCallback回调函数
        .OnWifiScanStateChanged = WifiScanStateCallback };
    osDelay(10);
    /* 使用RegisterWifiEvent接口，注册WiFi事件监听器 */ 
    errCode = RegisterWifiEvent(&eventListener);
    printf("[INFO] RegisterWifiEvent: %d\r\n", errCode);
    while(1)
    {
        /* 开启WiFi设备的STA模式 */
        errCode = EnableWifi();
        printf("[INFO] EnableWifi: %d\r\n", errCode);
        osDelay(100);
        /**
         * 开始扫描WiFi热点，只是触发扫描动作，并不会等到扫描完成才返回
         * 因此我们:
         * 1.定义了g_scanDone全局变量判断是否扫描得到热点
         * 2.定义了WifiPrintScanResult函数，通过GetScanInfoList API获取扫描结果
         */
        g_scanDone = 0;
        errCode = Scan();
        printf("[INFO] Scan: %d\r\n", errCode);
        /* 等待扫描完成 */ 
        while (!g_scanDone)
        {
            osDelay(5);
        }
        /* 处理扫描结果 */
        WifiPrintScanResult();
        /* 关闭WiFi设备的STA模式 */ 
        errCode = DisableWifi();
        printf("[INFO] DisableWifi: %d\r\n", errCode);
        printf("\r\n");
        osDelay(500);
    }
}


/* 入口函数 */
static void WifiScanEntry(void)
{
     osThreadAttr_t attr = {
        .name = "WifiScanTask",
        .stack_size = 10240,
        .priority = osPriorityNormal
    };
    if (osThreadNew(WifiScanTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

// SYS_RUN(WifiScanEntry);