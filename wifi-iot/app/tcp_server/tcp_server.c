#include <stdio.h>
#include "ohos_init.h"
#include "ohos_types.h"
#include "cmsis_os2.h"
// socket函数
#include "lwip/sockets.h"
// 字节序转换函数
#include "lwip/def.h"
#include "tcp_server.h"

#define BACKLOG 1

static char response[128] = "";


void TcpServerDemo(unsigned short port)
{
    // 返回值，发送/接收的字节数
    ssize_t retval = 0;

    // 创建一个 TCP Socket(网络套接字)，返回文件描述符
    // 用于监听客户端的连接请求
    int sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);

    // 记录客户端的IP地址和端口号
    struct sockaddr_in clientAddr = {0};

    // 配置服务端的地址信息
    struct sockaddr_in servertAddr = {
        // 选择IPv4协议族
        .sin_family = AF_INET,
        // 端口号，使用htons函数从主机字节序转为网络字节序
        // 大端序是网络标准字节序（建议转换保持兼容性）
        .sin_port = lwip_htons(port),
        // 允许任意主机接入，0.0.0.0
        // 使用htonl函数将32位整数从主机字节序转换为网络字节序
        .sin_addr.s_addr = lwip_htonl(INADDR_ANY), // 宏定义在inet.h
    };

    // 将sockfd与服务器IP、端口号绑定
    retval = lwip_bind(sockfd, (struct sockaddr *)&servertAddr, sizeof(servertAddr));
    if (retval < 0) {
        LOG_ERROR("bind fail, %ld!", retval);
        // 跳转到cleanup部分，主要是关闭连接
        goto do_cleanup;
    }
    LOG_INFO("bind server port %d success!", port);

    // 开始监听，最大等待队列长度为BACKLOG
    retval = lwip_listen(sockfd, BACKLOG);
    if (retval < 0) {
        LOG_ERROR("listen fail, %ld!", retval);
        // 跳转到cleanup部分，主要是关闭连接
        goto do_cleanup;
    }
    LOG_INFO("listen server port %d success!", port);

    // 阻塞式的等待客户端连接
    socklen_t clientAddrLen = sizeof(clientAddr);
    int connfd = lwip_accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (connfd < 0) {
        LOG_ERROR("accept fail, %d!", connfd);
        // 跳转到cleanup部分，主要是关闭连接
        goto do_cleanup;
    }
    // inet_ntoa函数在inet.h，lwip_ntohs在def.h
    // 将客户端的IP地址和端口号转换为字符串格式
    LOG_INFO("accept client %s:%d success!", inet_ntoa(clientAddr.sin_addr), lwip_ntohs(clientAddr.sin_port));

    // 使用默认阻塞模式接收数据
    retval = lwip_recv(connfd, response, sizeof(response), 0);
    // 对方的通信端关闭时，返回值为0；返回值小于0表示接收失败
    if (retval <= 0 ) {
        LOG_ERROR("recv fail, %ld!", retval);
        goto do_disconnect;
    }
    LOG_INFO("recv response {%s} from client", response);


    // 使用默认阻塞模式发送数据
    retval = lwip_send(connfd, response, sizeof(response), 0);
    // 对方的通信端关闭时，返回值为0；返回值小于0表示接收失败
    if (retval <= 0 ) {
        LOG_ERROR("send fail, %ld!", retval);
        goto do_disconnect;
    }
    LOG_INFO("send request {%s} to server %ld", response, retval);

do_disconnect:
    LOG_WARN("do_cleanup...");
    // 关闭Socket
    lwip_close(connfd);

do_cleanup:
    LOG_WARN("do_cleanup...");
    // 关闭Socket
    lwip_close(sockfd);
}