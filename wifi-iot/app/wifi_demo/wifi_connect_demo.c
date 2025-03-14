#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
// HAL接口API
#include "wifi_device.h"
// 海思SDK接口
// lwIP TCP/IP协议栈：网络接口API
// netifapi: Network Interface API
#include "lwip/netifapi.h"
// lwIP TCP/IP协议栈：SHELL命令API
#include "lwip/api_shell.h"
#include "lwip/sockets.h"
/**
 * wifi_demo/wifi_connect_demo.c
 * 连接Wi-Fi热点案例
 * -------------------------------------------------------------------------------
 * 第1步，使用`RegisterWifiEvent`接口注册 Wi-Fi 事件监听器。
 * 第2步，使用`EnableWifi`接口开启 Wi-Fi 设备的STA模式。
 * 第3步，使用`AddDeviceConfig`接口向系统添加热点配置，主要是 SSID、PSK 和加密方式等配置项。
 * 第4步，使用`ConnectTo`接口连接到热点上。
 * 第5步，在连接状态变化（`OnWifiConnectionChanged`）事件的回调函数中监测连接是否成功。
 * 第6步，等待连接成功。
 * 第7步，使用海思SDK接口的 DHCP 客户端API，从热点中获取IP地址。
 */

 // 全局变量，用于标识连接是否成功
static int g_connected = 0;

/**
 * @brief 打印Wifi连接信息
 * 
 * @param info 传入Wifi连接信息，WifiLinkedInfo类型
 */
static void WifiPrintConnInfo(WifiLinkedInfo *info)
{
    // 存储MAC地址字符串
    static char macAddress[32] = {0};
    // 获取MAC地址
    unsigned char *mac = info->bssid;
    // mac地址转换为字符串
    snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    // 打印信息
    printf("[INFO] %d,%d,%d,%d,%d,%d,%s\r\n",
        info->rssi,
        info->band,
        info->frequency,
        info->connState,
        info->disconnectedReason,
        info->ipAddress,
        macAddress
    );
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
    printf("[CALLBACK] %s %d,state:%d\r\n", __FUNCTION__, __LINE__, state);
    // 打印Wifi连接信息
    WifiPrintConnInfo(info);
    // 更新连接状态(全局变量)，WIFI_STATE_AVALIABLE 声明在 wifi_event.h
    if (state == WIFI_STATE_AVALIABLE) {
        g_connected = 1;
    } else {
        g_connected = 0;
    }
}

/**
 * @brief 扫描状态变化回调函数
 * 
 * @param state 扫描状态
 * @param size 扫描到的热点个数
 */
static void WifiScanStateCallback(int state, int size)
{
    // 输出日志
    printf("[CALLBACK] %s %d, state = %X, size = %d\r\n", __FUNCTION__, __LINE__, state, size);
}

/* 任务函数 */
static void WifiConnectTask(void* params)
{
    /* 定义Wifi错误代码接收变量 */
    WifiErrorCode errCode;
    /* 创建Wifi事件监听器 */
    WifiEvent eventListener = {
        .OnWifiConnectionChanged = WifiConnStateCallback,
        .OnWifiScanStateChanged = WifiScanStateCallback,
    };
    osDelay(10);
    /* 注册WiFi事件监听器 */
    errCode = RegisterWifiEvent(&eventListener);
    printf("[INFO] RegisterWifiEvent: %d\r\n", errCode);
    /* 定义要连接的指定Wifi的初始化*/
    WifiDeviceConfig apConfig = {
        // 热点名称
        .ssid = "OpenHarmony",
        // 热点密码
        .preSharedKey = "123456789",
        // 加密方式(PSK)
        .securityType = WIFI_SEC_TYPE_PSK,
    };
    // 接收networkId
    int netId;
    /* 主循环 */
    while(1)
    {
        // 使能station 模式.
        errCode = EnableWifi();
        printf("[INFO] EnableWifi: %d\r\n", errCode);
        // 添加热点的初始化
        errCode = AddDeviceConfig(&apConfig, &netId);
        printf("[INFO] AddDeviceConfig: %d\r\n", errCode);
        // 连接到热点
        ConnectTo(netId);
        // 等待连接
        while(!g_connected)
        {
            osDelay(10);
        }
        printf("[INFO] g_connected: %d\r\n", g_connected);
        osDelay(50);
        // 连接成功后，调用DHCP客户端接口，从热点获取IP地址
        // 获取STA模式的网络接口
        struct netif *iface = netifapi_netif_find("wlan0");
        // 获取网络接口成功
        if (iface) {
            // 启动DHCP客户端
            err_t ret = netifapi_dhcp_start(iface);
            printf("[INFO] netifapi_dhcp_start: %d\r\n", ret);
            // 等待DHCP服务端分配IP地址
            osDelay(200);
        }
        // 模拟一段时间的联网业务
        int timeout = 60;
        printf("[Warning] after %d seconds, I'll disconnect WiFi!\n", timeout);
        while (timeout--)
        {
            osDelay(100);
        }
        // 断开热点之前需要停止DHCP客户端
        err_t ret = netifapi_dhcp_stop(iface);
        printf("[INFO] netifapi_dhcp_start: %d\r\n", ret);
        // 断开热点
        Disconnect();
        printf("[INFO] disconnect!\r\n");
        // 删除热点配置
        RemoveDevice(netId);
        // 关闭WiFi设备的STA模式
        errCode = DisableWifi();
        printf("DisableWifi: %d\r\n", errCode);
        // 等待2秒
        osDelay(200);
    }
}

/* 入口函数 */
static void WifiScanEntry(void)
{
     osThreadAttr_t attr = {
        .name = "WifiConnectTask",
        .stack_size = 10240,
        .priority = osPriorityNormal
    };
    if (osThreadNew(WifiConnectTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

// SYS_RUN(WifiScanEntry);