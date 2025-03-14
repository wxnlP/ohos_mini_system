#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
// STA模式头文件
#include "wifi_connecter.h"
#include "mqtt_task.h"
#include "cJSON.h"

/**
 * mqtt_demo/mqtt_demo.c
 * MQTT示例
 */

/* WiFi信息 */
// 账号
#define HOTSPOT_SSID        "OpenHarmony"
// 密码
#define HOTSPOT_PASSWD      "123456789"
// 加密方式
#define HOTSPOT_TYPE        WIFI_SEC_TYPE_PSK
/* 华为云信息 */
// MQTT连接参数               
#define   MQTT_HOST             "9dfa5c258d.st1.iotda-device.cn-east-3.myhuaweicloud.com"     
#define   MQTT_PORT             1883
#define   MQTT_CLIENT_ID        "677388cfbab900244b135588_DATAS_0_0_2024123106"
#define   MQTT_USERNAME         "677388cfbab900244b135588_DATAS"
#define   MQTT_PASSWD           "ad31747e0b97a6ca1287043ca46a3571796f0d20e1056ecabec4f05fc2db545f"
#define   MQTT_DEVICE_ID        "677388cfbab900244b135588_DATAS"
#define   MQTT_SERVICE_ID       "Upload"
// HUAWEICLOUDE平台的话题定义  
#define   MQTT_PublishTopic     "$oc/devices/677388cfbab900244b135588_DATAS/sys/properties/report"
#define   MQTT_RequestTopic     "$oc/devices/677388cfbab900244b135588_DATAS/sys/shadow/get/request_id={request_id}"
#define   MQTT_SubscribeTopic   "$oc/devices/677388cfbab900244b135588_DATAS/sys/shadow/get/response/#"
// 温湿度数据
static float g_temp = 31;
static float g_humidity = 80; 
static int g_mq2 = 1;

/**
 * @brief 发布消息，上传温湿度、MQ2数据
 * 
 * @param temp 温度
 * @param humidity 湿度
 * @param mq2 MQ2是否超阈值
 * @return 形成的JSON发送格式
 */
static char* MqttPublishPayload(float temp, float humidity, int mq2) {
    // 创建JSON结构
    cJSON *root = cJSON_CreateObject();
    cJSON *services = cJSON_CreateArray();
    
    // 构建服务节点
    cJSON *upload = cJSON_CreateObject();
    cJSON_AddStringToObject(upload, "service_id", "Upload");
    
    // 添加属性
    cJSON *props = cJSON_CreateObject();
    cJSON_AddNumberToObject(props, "temperature", temp);
    cJSON_AddNumberToObject(props, "humidity", humidity);
    cJSON_AddNumberToObject(props, "MQ2", mq2);
    cJSON_AddItemToObject(upload, "properties", props);
    
    // 组合结构
    cJSON_AddItemToArray(services, upload);
    cJSON_AddItemToObject(root, "services", services);
    
    // 生成字符串
    char *payload = cJSON_PrintUnformatted(root);
    
    // 释放cJSON树（注意：不释放payload字符串）
    cJSON_Delete(root);
    
    return payload;
}

/**
 * @brief 
 * 
 * @param device_id 
 * @param service_id 
 * @return char* 
 */
static char* MqttRequestPayload(const char* device_id, const char* service_id) {
    // 参数有效性检查
    if (!device_id || !service_id || strlen(device_id) == 0 || strlen(service_id) == 0) {
        return NULL;
    }

    // 创建JSON根对象
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;

    // 添加设备ID字段
    cJSON_AddStringToObject(root, "object_device_id", device_id);
    // 添加服务ID字段
    cJSON_AddStringToObject(root, "service_id", service_id);

    // 生成紧凑型JSON
    char *payload = cJSON_PrintUnformatted(root);
    
    // 清理cJSON结构
    cJSON_Delete(root);
    
    return payload;
}


static void MqttTaskDemo(void* arg)
{
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
        printf("[ERROR] Connect to AP failed!\r\n");
        return;
    }
    /* 初始化并启动MQTT任务，连接MQTT服务器 */
    MqttTaskInit(); 
    if (MqttTaskConnect(MQTT_HOST, MQTT_PORT, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWD) != 0) {
        // 连接失败，输出错误信息并退出
        printf("[ERROR] Connect to MQTT server failed!\r\n");
        return;
    }
    /* 订阅主题 */ 
    int rc = MqttTaskSubscribe(MQTT_SubscribeTopic); 
    if (rc != 0) {
        // 订阅失败，输出错误信息并退出
        printf("[ERROR] MQTT Subscribe failed!\r\n");
        return;
    } else {
        // 输出订阅成功信息
        printf("[INFO] MQTT Subscribe OK\r\n");
    }

    /* 发布请求信息 */
    char* payload = MqttRequestPayload(MQTT_DEVICE_ID, MQTT_SERVICE_ID); 
    rc = MqttTaskPublish(MQTT_RequestTopic, payload); 
    if (rc != 0) {
        // 发布失败，输出错误信息
        printf("[ERROR] MQTT Request failed!\r\n"); 
        return;
    } else {
        // 发布成功，输出成功信息
        printf("[INFO] MQTT Request OK\r\n"); 
    }

    /* 发布消息 */
    payload = MqttPublishPayload(g_temp, g_humidity, g_mq2); 
    rc = MqttTaskPublish(MQTT_PublishTopic, payload); 
    if (rc != 0) {
        // 发布失败，输出错误信息
        printf("[ERROR] MQTT Publish failed!\r\n"); 
        return;
    } else {
        // 发布成功，输出成功信息
        printf("[INFO] MQTT Publish OK\r\n"); 
    }                  
}

/* 入口函数 */
static void MqttEntry(void)
{
     osThreadAttr_t attr = {
        .name = "MqttTaskDemo",
        .stack_size = 10240,
        .priority = osPriorityNormal
    };
    if (osThreadNew(MqttTaskDemo, NULL, &attr) == NULL) {
        printf("[ERROR] Thread Create Faild.\r\n");
    }
}

SYS_RUN(MqttEntry);