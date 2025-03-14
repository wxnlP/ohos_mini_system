/* 新增组件示例 */
# include <stdio.h>
// OpenHarmony特有头文件
# include "ohos_init.h"

// 定义函数
void entry(void)
{
    printf("This is a component.\r\n");
}

// 让entry函数在系统启动的第四阶段即system startup阶段，以优先级为 2 的级别执行
// SYS_RUN(entry);