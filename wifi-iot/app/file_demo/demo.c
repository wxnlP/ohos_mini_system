#include <stdio.h>
#include <string.h>     
#include "ohos_init.h" 
#include "utils_file.h" 


void file_demo(void)
{
    char filename[] = "wifi_INFO";
    unsigned int fileslen=0;
    int ret = UtilsFileStat(filename, &fileslen);
    if (ret == -1)
    {
        // 创建写入的数据
        const char* dataWrite= "openharmony,123456789";
        printf("%s文件不存在。\n", filename);
        // 打开文件
        int fd = UtilsFileOpen(filename, O_WRONLY_FS|O_CREAT_FS|O_TRUNC_FS, 0);
        printf("OpenFile --> result: %d\n", fd);
        // 写入数据
        int data_len = UtilsFileWrite(fd, dataWrite, strlen(dataWrite));
        printf("WriteFile --> result: %d\n", data_len);
        // 关闭文件
        UtilsFileClose(fd);
    }
    else
    {
        printf("%s文件存在。\n", filename);
         // 打开文件，读写模式
        int fd = UtilsFileOpen(filename, O_RDWR_FS, 0);
        printf("OpenFile --> result: %d\n", fd);
        // 定位读写偏移量
        ret = UtilsFileSeek(fd, 0, SEEK_SET_FS);
        printf("SeekFile --> result: %d\n", ret);
        // 读取数据
        char ssid[32] = {};
        int data_len1 = UtilsFileRead(fd, ssid, 11);
        printf("ReadFile --> result: %d ssid: %s\n", data_len1, ssid);
        // 重新定位读写偏移量
        ret = UtilsFileSeek(fd, 1, SEEK_CUR_FS);
        printf("SeekFile --> result: %d\n", ret);
        // 读取数据
        char passwd[32] = {};
        int data_len2 = UtilsFileRead(fd, passwd, 9);
        printf("ReadFile --> result: %d passwd: %s\n", data_len2, passwd);
        // 关闭文件
        UtilsFileClose(fd);

        // 删除文件
        ret = UtilsFileDelete(filename);
        printf("DeleteFile --> result: %d\n", ret);
    }
    
    
}


// APP_FEATURE_INIT(file_demo);