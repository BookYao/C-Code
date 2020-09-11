
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "msgPacket.h"
#include "cJSON.h"

int msgTextParse(char *msgText, T_MsgText *msgJson) {
    if (!msgText || !msgText[0] || !msgJson) {
        printf("%s.%d. msgTextParse Param error!", __FUNCTION__, __LINE__);
        return -1;
    }

    cJSON  *root = cJSON_Parse((const char *) msgText);
    if (!root)
    {
        printf("%s:%d. msgTextParse is error: %s", 
                        __FUNCTION__, __LINE__, (char *)msgText);
        return  -1;
    }
    
    cJSON  *item = NULL;
    item = cJSON_GetObjectItem(root, "identity");
    if (item && item->valuestring) {
        strncpy(msgJson->identity, item->valuestring, 
                sizeof(msgJson->identity) - 1);
    } else {
        printf("%s:%d. msgTextParse identity data is error: %s", 
                        __FUNCTION__, __LINE__, (char *)msgText);
        cJSON_Delete(root);
        return  -1;
    }
    
    cJSON_Delete(root);
    return 0;
}

int makeMilliTimestamp(char *timestamp, int buflen) {
    if (!timestamp || buflen < 0) {
        printf("%s.%d. makeMilliTimestamp Param error!", __FUNCTION__, __LINE__);
        return -1; 
    }   

    struct timeval tv; 
    memset(&tv, '\0', sizeof(struct timeval));

    gettimeofday(&tv, NULL);
    snprintf(timestamp, buflen - 1, "%ld%ld", tv.tv_sec, tv.tv_usec/1000);
    return 0;
}

int buildSignUpOfflineMsg(T_MsgText *msgText, char *signupOfflineMsg, int len) {
    cJSON  *root, *fmt, *array;
    char *out = NULL;
    char milliTimestamp[16] = {'\0'};

    makeMilliTimestamp(milliTimestamp, sizeof(milliTimestamp));
    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "result", array = cJSON_CreateArray());

    fmt = cJSON_CreateObject();
    cJSON_AddItemToObject(fmt, "identity", cJSON_CreateString((char *) msgText->identity));
    cJSON_AddItemToObject(fmt, "result", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(fmt, "time", cJSON_CreateString((char *)milliTimestamp));
    cJSON_AddItemToObject(fmt, "longitude", cJSON_CreateNull());
    cJSON_AddItemToObject(fmt, "latitude", cJSON_CreateNull());
    cJSON_AddItemToObject(fmt, "address", cJSON_CreateNull());
    cJSON_AddItemToArray(array, fmt);

    out = cJSON_Print(root);
    cJSON_Delete(root);

    strncpy(signupOfflineMsg, out, len - 1);

    free(out);
    return 0;
}

int parseHttpResMsg(char *msgText, T_HttpResMsg *resMsg) {
    cJSON  *root = cJSON_Parse((const char *) msgText);
    if (!root) {
        printf("%s:%d. msgTextParse is error: %s", 
                        __FUNCTION__, __LINE__, (char *)msgText);
        return  -1;
    }
    
    cJSON  *item = NULL;
    item = cJSON_GetObjectItem(root, "code");
    if (item && item->valuestring) {
        strncpy(resMsg->code, item->valuestring, 
                sizeof(resMsg->code) - 1);
    } else {
        printf("%s:%d. parseHttpResMsg code is error: %s", 
                        __FUNCTION__, __LINE__, (char *)msgText);
        cJSON_Delete(root);
        return  -1;
    }

    item = cJSON_GetObjectItem(root, "message");
    if (item && item->valuestring) {
        strncpy(resMsg->message, item->valuestring, 
                sizeof(resMsg->message) - 1);
    } else {
        printf("%s:%d. parseHttpResMsg message is error: %s", 
                        __FUNCTION__, __LINE__, (char *)msgText);
        cJSON_Delete(root);
        return  -1;
    }
    item = cJSON_GetObjectItem(root, "success");
    if (item && item->valuestring) {
        strncpy(resMsg->success, item->valuestring, 
                sizeof(resMsg->success) - 1);
    } else {
        printf("%s:%d. parseHttpResMsg success is error: %s", 
                        __FUNCTION__, __LINE__, (char *)msgText);
        cJSON_Delete(root);
        return  -1;
    }

    item = cJSON_GetObjectItem(root, "data");
    if (item && item->valuestring) {
        strncpy(resMsg->data, item->valuestring, 
                sizeof(resMsg->data) - 1);
    } else {
        printf("%s:%d. parseHttpResMsg data field is error: %s", 
                        __FUNCTION__, __LINE__, (char *)msgText);
        cJSON_Delete(root);
        return  -1;
    }
    
    cJSON_Delete(root);
    return 0;
}

