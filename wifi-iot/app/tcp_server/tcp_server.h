#ifndef TCP_SERVER
#define TCP_SERVER

/* 定义日志宏 */
#define LOG_INFO(format, ...) printf("[INFO] [%s] "format"\n",\
                                        __FUNCTION__,\
                                        ##__VA_ARGS__)
#define LOG_ERROR(format, ...) printf("[ERROR] [%s:%d] "format"\n",\
                                        __FUNCTION__,\
                                        __LINE__,\
                                        ##__VA_ARGS__)
#define LOG_WARN(format, ...) printf("[WARN] [%s:%d] "format"\n",\
                                        __FUNCTION__,\
                                        __LINE__,\
                                        ##__VA_ARGS__)

void TcpServerDemo(unsigned short port);

#endif