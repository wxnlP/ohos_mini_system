# include <stdio.h>
# include "ohos_init.h"
# include "kv_store.h"

/*
  functions:
    Untils
*/
const char* key = "name";
char* value[32] = {};


void kv_storeTest(void)
{
    int retGet = UtilsGetValue(key, value, 32);
    if (retGet < 0)
    {
        const char* valueToWrite = "OpenHarmony.";
        int retSet = UtilsSetValue(key, valueToWrite);
        printf("SetValue --> result: %d\n", retSet);
    }
    else
    {
        printf("GetValue --> result: %d  value: %s\n", retGet, value);
    }

}


// APP_FEATURE_INIT(kv_storeTest);