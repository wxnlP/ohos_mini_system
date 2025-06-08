#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "cmsis_os2.h"
#include "wifi_connecter.h"
#include "tcp_server.h"

/* WiFi信息 */
// 账号
#define  HOTSPOT_SSID           "CMCC-XPeA"
// 密码 
#define  HOTSPOT_PASSWD         "cyjj7346"
// 加密方式 
#define  HOTSPOT_TYPE           WIFI_SEC_TYPE_PSK
// 用于标识TCP服务器端口
#define PARAM_SERVER_PORT 5678


static void TcpServerTask(void *arg)
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

    TcpServerDemo(PARAM_SERVER_PORT);

    LOG_INFO("disconnect ap ...");
    DisconnectWithHotspot(netId);
    LOG_INFO("disconnect ap done");

}

static void TcpServerEntry(void)
{
    osThreadAttr_t attr = {
        .name = "TcpServerTask",
        .stack_size = 10240,
        .priority = osPriorityNormal,
    };
    osThreadId_t thread_id = osThreadNew(TcpServerTask, NULL, &attr);
    if (thread_id == NULL) {
        printf("[Thread Create] osThreadNew(%s) failed.\r\n", "TcpServerTask");
    } else{
        printf("[Thread Create] osThreadNew(%s) success, thread id: %d.\r\n", "TcpServerTask", thread_id);
    }
}

SYS_RUN(TcpServerEntry);