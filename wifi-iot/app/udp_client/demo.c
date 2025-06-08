#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "cmsis_os2.h"
#include "wifi_connecter.h"
#include "udp_client.h"

/* WiFi信息 */
// 账号
#define  HOTSPOT_SSID           "OpenHarmony"
// 密码 
#define  HOTSPOT_PASSWD         "123456789"
// 加密方式 
#define  HOTSPOT_TYPE           WIFI_SEC_TYPE_PSK
// TCP服务器IP地址
#define PARAM_SERVER_ADDR "192.168.10.117"
// 用于标识TCP服务器端口
#define PARAM_SERVER_PORT 5678


static void UdpClientTask(void *arg)
{
    (void)arg;
    /* 初始化WIFI参数 */
    WifiDeviceConfig apConfig = {
        // 热点名称
        .ssid = HOTSPOT_SSID,
        // 热点密码
        .preSharedKey = HOTSPOT_PASSWD,
        // 加密方式(PSK)
        .securityType = HOTSPOT_TYPE,
    };

    /* 连接WIFI */
    int netId = ConnectToHotspot(&apConfig);
    if (netId < 0) {
        LOG_ERROR("Connect to AP failed!");
    }

    UdpClientDemo(PARAM_SERVER_ADDR, PARAM_SERVER_PORT);

    LOG_INFO("disconnect ap ...");
    DisconnectWithHotspot(netId);
    LOG_INFO("disconnect ap done");

}

static void UdpClientEntry(void)
{
    osThreadAttr_t attr = {
        .name = "UdpClientTask",
        .stack_size = 10240,
        .priority = osPriorityNormal,
    };
    osThreadId_t thread_id = osThreadNew(UdpClientTask, NULL, &attr);
    if (thread_id == NULL) {
        printf("[Thread Create] osThreadNew(%s) failed.\r\n", "UdpClientTask");
    } else{
        printf("[Thread Create] osThreadNew(%s) success, thread id: %d.\r\n", "UdpClientTask", thread_id);
    }
}

SYS_RUN(UdpClientEntry);