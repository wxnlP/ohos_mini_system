# include <stdio.h>
# include "ohos_init.h"
# include "component_1.h"
# include "component_2.h"
# include "../driver/drv_1.h"
# include "../library/lib_1.h"
# include "../library/lib_2.h"

void myapp(void){

    
    printf("app runs!\r\n");
}

// 让myapp在系统启动的第八个阶段以优先级 2 执行
// APP_FEATURE_INIT(myapp);



