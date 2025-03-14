#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cmsis_os2.h"
// MQTTClient-C库接口文件
#include "MQTTClient.h"
// OHOS(LiteOS)适配接口文件
#include "mqtt_ohos.h"
// 自定义的接口文件
#include "mqtt_task.h"


// 定义一个宏，用于输出日志
#define LOGI(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

// MQTT客户端(MQTTClient.h)
static MQTTClient client = {0};

// MQTT网络连接(mqtt_ohos.h)
static Network network = {0};

// 接收和发送数据的缓冲区
static unsigned char sendbuf[512], readbuf[512];

// 标识任务循环运行与否
static int running = 1;

/**
 * @brief MQTT任务循环
 * 
 * @param arg MQTT客户端(MQTTClient)
 */
static void MqttTask(void* arg)
{
    /* 输出开始日志 */
    LOGI("[INFO] MqttTask start!");
    /* 获取任务参数 */
    MQTTClient* pClient = (MQTTClient*)arg;
    /* 任务循环 */
    while (pClient)
    {
        // paho_mqtt对互斥锁操作进行了一个简单的封装
        // 当宏 MQTT_TASK 被定义后，MQTTClient结构体会多两个成员 mutex 和 thread

        /* 获取互斥锁(mqtt_ohos_cmsis.c) */
        mqttMutexLock(&pClient->mutex);
        if (!running) {
            // 退出任务循环 
            LOGI("[ERROR] MQTT background thread exit!");
            /* 释放互斥锁 */
            mqttMutexUnlock(&pClient->mutex);
            break;
        }
        /* 释放互斥锁(mqtt_ohos_cmsis.c) */
        mqttMutexUnlock(&pClient->mutex);
        /* ---------------------------------------- */
        /* 获取互斥锁 */
        mqttMutexLock(&pClient->mutex);
        // 客户端连接成功
        if (pClient->isconnected) {
            // 维持 MQTT 客户端的后台通信(MQTTClient.h)
            MQTTYield(pClient, 100);
        }
        /* 释放互斥锁 */
        mqttMutexUnlock(&pClient->mutex);
        /* ---------------------------------------- */
        // 等待 1 s(mqtt_ohos_cmsis.c)
        Sleep(1000);
    }
    // 输出日志
    LOGI("[ERROR] MqttTask exit!");
}


/**
 * @brief 初始化 MqttTask,基于 MqttTask 创建一个线程
 * 
 */
void MqttTaskInit(void)
{
    /* 初始化并启动MQTT客户端 */
    // 网络初始化(mqtt_ohos_cmsis.c)
    NetworkInit(&network);
    // 客户端初始化(MQTTClient.h)
    MQTTClientInit(&client, &network, 100, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    /* 创建MQTT线程 */
    // paho_mqtt对创建线程操作进行了一个简单的封装
    ThreadStart(&client.thread, MqttTask, &client);
    LOGI("[INFO] MqttTaskInit done!");
}

/**
 * @brief 停止MQTT任务
 * 
 */
void MqttTaskDeinit(void)
{
    // 获取互斥锁
    mqttMutexLock(&client.mutex);
    // 标识变量归零
    running = 0;
    // 释放互斥锁
    mqttMutexUnlock(&client.mutex);
    // 删除互斥锁
    mqttMutexDeinit(&client.mutex);
}

/**
 * @brief 连接MQTT服务器(Broker)
 * 
 * @param host 服务器地址
 * @param port 服务器端口
 * @param clientId 客户端ID
 * @param username 用户名
 * @param password 密码
 * @return 0：成功，-1：失败
 */
int MqttTaskConnect(const char *host, unsigned short port, 
                    const char *clientId, const char *username, const char *password)
{
    /* 接收返回值变量 */
    int rc = 0;

    /* 初始化MQTT连接信息(MQTTPacket\MQTTConnect.h) */
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    /* 使用TCP socket连接MQTT服务器 */
    rc = NetworkConnect(&network, (char*)host, port);
    if (rc != 0) {
        // 连接失败，输出日志并返回-1
        LOGI("[ERROR] NetworkConnect is %d", rc);
        return -1;
    }

    /* 设置MQTT连接信息 */
    if (username != NULL && password != NULL) {
        connectData.username.cstring = (char*)username;
        connectData.password.cstring = (char*)password;
        // MQTT版本，3 = 3.1，4 = 3.1.1
        connectData.MQTTVersion = 3;
        connectData.clientID.cstring = (char*)clientId;
    }

    /* 发送MQTT连接包(MQTTClient.h) */
    rc = MQTTConnect(&client, &connectData);
    if (rc != 0) {
        // 连接失败，输出日志并返回-1
        LOGI("[ERROR] MQTTConnect failed: %d", rc);
        return -1;
    }

    /* 连接成功 */
    LOGI("[INFO] MQTT Connected!");
    return 0;
}

/* 主题订阅回调函数 */
static void MessageHander(MessageData* data)
{
    // (MQTTClient.h)
    printf("[INFO] Message Size:%d\r\n", (int)data->message->payloadlen);
    LOGI("[INFO] Message arrived on topic:\r\nTopic: %.*s\r\n",
        (int)data->topicName->lenstring.len, (char *)data->topicName->lenstring.data);
    LOGI("[INFO] payload: %.*s\r\n", (int)data->message->payloadlen, (char *)data->message->payload);
}

/**
 * @brief 订阅主题
 * 
 * @param topic 主题
 * @return 0：成功，-1：失败
 */
int MqttTaskSubscribe(char* topic)
{
    /* 接收返回值变量 */
    int rc = 0;

    /* 输出日志 */
    LOGI("[INFO] Subscribe: [%s] from broker", topic);

    /* 发送订阅包 */
    rc = MQTTSubscribe(&client, topic, QOS0, MessageHander);
    if (rc != 0) {
        // 连接失败，输出日志并返回-1
        LOGI("[ERROR] MQTTSubscribe failed: %d", rc);
        return -1;
    }

    /* 订阅成功 */
    return 0;
}

/**
 * @brief 取消主题订阅
 * 
 * @param topic 主题
 * @return 0：成功，-1：失败
 */
int MqttTaskUnSubscribe(char* topic)
{
    /* 接收返回值变量 */
    int rc = 0;

    /* 输出日志 */
    LOGI("[INFO] UnSubscribe: [%s] from broker", topic);

    /* 发送订阅包 */
    rc = MQTTUnsubscribe(&client, topic);
    if (rc != 0) {
        // 连接失败，输出日志并返回-1
        LOGI("[ERROR] MQTTUnsubscribe failed: %d", rc);
        return -1;
    }

    /* 取消订阅成功 */
    return 0;
}

/**
 * @brief 向指定主题发布消息
 * 
 * @param topic 主题
 * @param payload 
 * @return 0：成功，-1：失败
 */
int MqttTaskPublish(char *topic, char *payload)
{
    /* 接收返回值变量 */
    int rc = 0;

    /* 定义MQTT消息数据包(MQTTClient.h) */
    MQTTMessage message = {
        .qos = QOS0,
        .retained = 0,
        .payload = payload,
        .payloadlen = strlen(payload),
    };
    LOGI("[INFO] Publish: #'%s': '%s' to broker", topic, payload);

    /* 发布消息 */
    rc = MQTTPublish(&client, topic, &message);
    if (rc != 0) {
        // 连接失败，输出日志并返回-1
        LOGI("[ERROR] MQTTPublish failed: %d", rc);
        return -1;
    }

    /* 发布成功 */
    return 0;
}

/**
 * @brief 断开与MQTT服务器的连接
 * 
 * @return 0：成功，-1：失败
 */
int MqttTaskDisconnect(void)
{
    /* 接收返回值变量 */
    int rc = 0;

    /* 发送断开连接数据包 */
    rc = MQTTDisconnect(&client);
    if (rc != 0) {
        // 连接失败，输出日志并返回-1
        LOGI("[ERROR] MQTTDisconnect failed: %d", rc);
        return -1;
    }

    /* 断开和MQTT服务器的TCP socket连接 */
    NetworkDisconnect(&network);

    /* 断开连接成功 */
    return 0;
}
