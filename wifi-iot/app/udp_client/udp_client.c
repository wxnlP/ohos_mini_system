#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "cmsis_os2.h"
#include "lwip/sockets.h"
#include "lwip/def.h"
#include "udp_client.h"


/* 缓冲区 */
static char request[] = "hello";
static char response[128] = "";


/**
 * @brief Udp Client测试函数
 * 
 * lwip的API分两种，一中是带"_lwip"后缀的，一种则是不带后缀的。
 * 个人建议使用带后缀的，代码比较有区分度
 * 
 */
void UdpClientDemo(const char *host, unsigned short port)
{
    // 返回值，发送/接收的字节数
    ssize_t retval = 0;

    // ⭐创建一个 TCP Socket(网络套接字)，返回文件描述符
    int sockfd = lwip_socket(AF_INET, SOCK_DGRAM, 0); 

    // 设置服务端的地址信息，包括协议、端口、IP地址等
    struct sockaddr_in serverAddr = {
        // 选择IPv4协议
        .sin_family = AF_INET,
        // 端口号，使用htons函数从主机字节序转为网络字节序
        // 大端序是网络标准字节序（建议转换保持兼容性）
        .sin_port = lwip_htons(port),
    };

    // 将服务端IP地址从“点分十进制”字符串，转化为标准格式（32位整数）
    if (lwip_inet_pton(AF_INET, host, &serverAddr.sin_addr) <= 0) {
        LOG_ERROR("lwip_inet_pton fail!");
        // 跳转到cleanup部分，主要是关闭连接
        goto do_cleanup;
    }

    // ⭐尝试和目标主机建立连接，连接成功会返回0，失败返回-1
    // if (lwip_connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    //     LOG_ERROR("connect fail!");
    //     // 跳转到cleanup部分，主要是关闭连接
    //     goto do_cleanup;
    // }
    // 建立连接成功之后，sockfd就具有了“连接状态”，
    // 后续的发送和接收，都是针对指定的目标主机和端口
    // LOG_INFO("connect server %s success!", host);

    // ⭐使用默认阻塞模式发送数据
    retval = lwip_sendto(sockfd, request, sizeof(request), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    // 对方的通信端关闭时，返回值为0；返回值小于0表示接收失败
    if (retval <= 0 ) {
        LOG_ERROR("send fail, %ld!", retval);
        goto do_cleanup;
    }
    LOG_INFO("sendto request {%s} to server %ld", request, retval);

    // ⭐记录发送方的地址信息
    struct sockaddr_in fromAddr = {0};
    socklen_t fromLen = sizeof(fromAddr);
    // ⭐使用默认阻塞模式接收数据
    retval = lwip_recvfrom(sockfd, response, sizeof(response), 0, (struct sockaddr *)&fromAddr, &fromLen);
    // 对方的通信端关闭时，返回值为0；返回值小于0表示接收失败
    if (retval <= 0 ) {
        LOG_ERROR("recv fail, %ld!", retval);
        goto do_cleanup;
    }
    // 在末尾添加字符串结束符'\0'，以便后续的字符串操作
    response[retval] = '\0';
    LOG_INFO("recvfrom response {%s} to server %ld", response, retval);

    // ⭐显示发送方的地址信息
    LOG_INFO("peer info %s:%d success!", inet_ntoa(fromAddr.sin_addr), lwip_ntohs(fromAddr.sin_port));

    // 接收数据
do_cleanup:
    LOG_WARN("do_cleanup...");
    // 关闭Socket
    lwip_close(sockfd);
}

