#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
// HAL接口API
#include "wifi_hotspot.h"
// 海思SDK接口
// lwIP TCP/IP协议栈：网络接口API
// netifapi: Network Interface API
#include "lwip/netifapi.h"

/**
 * wifi_demo/wifi_hotspot_demo.c
 * 创建Wi-Fi热点案例
 * -------------------------------------------------------------------------------
 * 第1步，使用`RegisterWifiEvent`接口注册 Wi-Fi 事件监听器。
 * 第2步，准备AP的配置参数，包括SSID、PSK、加密方式、频带类型、信道等。
 * 第3步，使用`SetHotspotConfig`接口对系统设置当前热点的配置信息。
 * 第4步，使用`EnableHotspot`接口开启 Wi-Fi 设备的 AP 模式。
 * 第5步，在热点状态变化（`OnHotspotStateChanged`）事件的回调函数中，监测热点是否成功开启。
 * 第6步，等待热点成功开启。
 * 第7步，使用`netifapi_netif_set_addr`接口设置热点的IP地址、子网掩码、网关等信息。
 * 第8步，使用`netifapi_dhcps_start`接口启动 DHCP 服务。
 */

// 热点连接成功标识
static int g_hotspotStarted;
// 已经连接热点的站点个数
static int g_joinedStations;
// 网络接口
static struct netif *g_iface;

/**
 * @brief 打印连接热点的站点信息
 * 
 * @param info 站点信息
 */
static void HotspotPrintStaInfo(StationInfo *info)
{
    // 存储MAC地址字符串
    static char macAddress[32] = {0};
    // 获取MAC地址
    unsigned char *mac = info->macAddress;
    // mac地址转换为字符串
    snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("[CALLBACK] HotspotStaJoin: %s,%s\r\n",
        macAddress,
        info->name);
}

/**
 * @brief 热点状态变化回调函数
 * 
 * @param state 热点状态
 */
static void HotspotStateCallback(int state)
{
    /* 输出日志 */
    printf("[CALLBACK] OnHotspotStateChanged: %d.\r\n", state);
    /* 更新状态标识 */
    if (state == WIFI_HOTSPOT_ACTIVE) {
        g_hotspotStarted = 1;
    } else {
        g_hotspotStarted = 0;
    }
}

/**
 * @brief 站点加入热点回调函数
 * 
 * @param info 站点信息
 */
static void HotspotStaJoinCallback(StationInfo *info)
{
    g_joinedStations++;
    /* 打印连接站点的信息 */
    HotspotPrintStaInfo(info);
    printf("[CALLBACK] +StaJoinCallback: active stations = %d.\r\n", g_joinedStations);
}

/**
 * @brief 站点断开热点回调函数
 * 
 * @param info 站点信息
 */
static void HotspotStaLeaveCallback(StationInfo *info)
{
    g_joinedStations--;
    /* 打印连接站点的信息 */
    HotspotPrintStaInfo(info);
    printf("[CALLBACK] -StaLeaveCallback: active stations = %d.\r\n", g_joinedStations);
}

/**
 * @brief 打开热点
 * 
 * @param hsConfig 热点初始化
 * @param wifiEvent 事件监听绑定的回调函数
 * 
 */
static void HotspotStart(HotspotConfig *hsConfig, WifiEvent* wifiEvent)
{
    /* 接收错误代码 */
    WifiErrorCode errCode;
    /* 创建Wifi事件监听器 */
    errCode = RegisterWifiEvent(wifiEvent);
    printf("[INFO] RegisterWifiEvent: %d\r\n", errCode);
    /* 设置当前热点的配置信息 */
    errCode = SetHotspotConfig(hsConfig);
    printf("[INFO] SetHotspotConfig: %d\r\n", errCode);
    /* 开启AP模式 */
    // 初始化热点连接成功标识
    g_hotspotStarted = 0;
    errCode = EnableHotspot();
    printf("[INFO] EnableHotspot: %d\r\n", errCode);
    /* 等待热点开启成功 */
    while (!g_hotspotStarted)
    {
        osDelay(10);
    }
    printf("g_hotspotStarted = %d.\r\n", g_hotspotStarted);
    /**
     * @brief 热点开启成功之后，需要启动DHCP服务端，Hi3861使用如下接口：
     * 使用netifapi_netif_find("ap0")获取AP模式的网络接口
     * 使用netifapi_netif_set_addr接口设置热点本身的IP地址、网关、子网掩码
     * 使用netifapi_dhcps_start接口启动DHCP服务端
     * 使用netifapi_dhcps_stop接口停止DHCP服务端
     */
    // 获取网络接口
    g_iface = netifapi_netif_find("ap0");

    if (g_iface) {
        // 存储IP地址
        ip4_addr_t ipaddr;
        // 存储网关
        ip4_addr_t gateway;
        // 存储子网掩码
        ip4_addr_t netmask;
        // 设置IP地址
        IP4_ADDR(&ipaddr, 192, 168, 12, 1);    /* input your IP for example: 192.168.12.1 */
        // 设置子网掩码
        IP4_ADDR(&netmask, 255, 255, 255, 0); /* input your netmask for example: 255.255.255.0 */
        // 设置网关
        IP4_ADDR(&gateway, 192, 168, 12, 1);   /* input your gateway for example: 192.168.12.1 */
        // 设置热点的IP地址、子网掩码、网关
        err_t ret = netifapi_netif_set_addr(g_iface, &ipaddr, &netmask, &gateway);
        // 打印接口调用结果
        printf("[INFO] netifapi_netif_set_addr: %d\r\n", ret);
        // 停止DHCP服务 (DHCP服务有可能默认是开启状态)
        ret = netifapi_dhcps_stop(g_iface);
        // 打印接口调用结果
        printf("[INFO] netifapi_dhcps_stop: %d\r\n", ret);
        // 启动DHCP服务
        ret = netifapi_dhcps_start(g_iface, 0, 0);
        // 打印接口调用结果
        printf("[INFO] netifapi_dhcps_start: %d\r\n", ret);
    }
}

/**
 * @brief 关闭热点
 * 
 * @param wifiEvent 事件监听绑定的回调函数
 *
 */
static void HotspotStop(WifiEvent* wifiEvent)
{
    /* 如果之前已经成功获取网络接口 */
    if (g_iface) {
        // 停止DHCP服务
        err_t ret = netifapi_dhcps_stop(g_iface);
        // 打印接口调用结果
        printf("netifapi_dhcps_stop: %d\r\n", ret);
    }
    /* 接触Wifi事件监听 */
    WifiErrorCode errCode = UnRegisterWifiEvent(&wifiEvent);
    printf("[INFO] UnRegisterWifiEvent: %d\r\n", errCode);
    /* 关闭热点 */
    errCode = DisableHotspot();
    printf("[INFO] DisableHotspot: %d\r\n", errCode);
}

static void WifiHotspotTask(void* params)
{
    /* 接收错误代码 */
    WifiErrorCode errCode;
    /* 热点初始化 */
    HotspotConfig hsConfig = {
        .ssid = "BearPi",
        .preSharedKey = "openharmony",
        .securityType = WIFI_SEC_TYPE_PSK,
        .band = HOTSPOT_BAND_TYPE_2G,
        .channelNum = 7,
    };
    osDelay(10);
    /* 绑定回调函数 */
    WifiEvent eventListener = {
        // 热点状态变化
        .OnHotspotStateChanged = HotspotStateCallback,
        // 站点已连接
        .OnHotspotStaJoin = HotspotStaJoinCallback,
        // 站点断开连接
        .OnHotspotStaLeave = HotspotStaLeaveCallback,
    };
    /* 开启热点 */
    printf("[INFO] starting AP ...\r\n");
    HotspotStart(&hsConfig, &eventListener);
    printf("[INFO] StartHotspot: %d\r\n", errCode);
    /* 热点将开启1分钟 */
    int timeout = 60;
    printf("After %d seconds Ap will turn off!\r\n", timeout);
    while (timeout--)
    {
        osDelay(100);
    }
    /* 关闭热点 */
    printf("[INFO] stop AP ...\r\n");
    HotspotStop(&eventListener);
    printf("[INFO] AP stopped.\r\n");
}

/* 入口函数 */
static void HotspotEntry(void)
{
     osThreadAttr_t attr = {
        .name = "WifiHotspotTask",
        .stack_size = 10240,
        .priority = osPriorityNormal
    };
    if (osThreadNew(WifiHotspotTask, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

// SYS_RUN(HotspotEntry);