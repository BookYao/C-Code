
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>

#include "msgPacket.h"
#include "httpInterface.h"
#include "cJSON.h"

#define LOGIN_MSG   "login_msg"

static char *httpUrl = "http://10.0.0.39:8003/api/vl/audio_attendance/result";

void signupMsgOfflineHandle(char *msgText, char *msgType) {
    int     ret = 0;
    char    signupOffLineMsg[256] = {'\0'};
    char    httpResInfo[4096] = {'\0'};
    T_MsgText   msgTextJson;
    T_HttpResMsg httpResMsg;
    
    if (strcmp(msgType, LOGIN_MSG) == 0) {
        memset(&msgTextJson, 0, sizeof(T_MsgText));
        if (0 != msgTextParse(msgText, &msgTextJson)) {
            printf("%s:%d. msgTextParse failed. Msgtext:%s", 
                        __FUNCTION__, __LINE__, (char *)msgText);
            return ;
        }
        
        buildSignUpOfflineMsg(&msgTextJson, signupOffLineMsg, sizeof(signupOffLineMsg));
        
        ret = httpPostData(httpUrl, signupOffLineMsg, strlen(signupOffLineMsg), httpResInfo);
        if (ret != HTTP_OK) {
            printf("%s:%d. UserOffline. HttpResponse Failed. Msgtext:%s-Httpret:%d\n-URL:%s", 
                            __FUNCTION__, __LINE__, (char *)signupOffLineMsg, ret, httpUrl);
            return ;
        }

        printf("%s:%d. HttpResponse Msgtext:%s", 
                      __FUNCTION__, __LINE__, httpResInfo);

        memset(&httpResMsg, '\0', sizeof(T_HttpResMsg));
        ret = parseHttpResMsg(httpResInfo, &httpResMsg);
        if (ret < 0) {
            printf("%s:%d. UserOffline. parseHttpResMsg Failed. Msgtext:%s", 
                            __FUNCTION__, __LINE__, (char *)httpResInfo);
            return ;
        }

        if (strcmp(httpResMsg.success, "true") != 0) {
            printf("%s:%d. HttpResMsg Handle Failed. HttpResMsg:%s", 
                            __FUNCTION__, __LINE__, (char *)httpResInfo);
            return ;
        }        
    }
}

void buildTestMsg(char *buf, int buflen) {
    cJSON  *root;
    char *out = NULL;
    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "identity", cJSON_CreateString((char *) "13800001111"));

    out = cJSON_Print(root);
    cJSON_Delete(root);

    strncpy(buf, out, buflen - 1);

    free(out);
}

int main() {
    char msg[256] = {'\0'};
    buildTestMsg(msg, sizeof(msg));
    signupMsgOfflineHandle(msg, LOGIN_MSG);
    return 0;
}

