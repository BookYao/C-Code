

#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

#include "httpInterface.h"


char *getHttpErrorCode(int error)
{
    switch(error)
    {
        case HTTP_OK:
            return "HTTP OK!";
        case HTTP_UNSUPPORTED_PROTOCOL:
            return "Not Support Protocal, Please Set HTTP Head!";
        case HTTP_COULDNT_CONNECT:
            return "Failed To Connect Remote Host!";
        case HTTP_HTTP_RETURNED_ERROR:
            return "HTTP Return Error, Check Server!";
        case HTTP_READ_ERROR:
            return "HTTP Local Read Error!";
        case HTTP_INIT_ERROR:
            return "HTTP Init Error!";
        case HTTP_OTHER_ERROR:
            return "User Client Data Error!";
    }

    return NULL;
}


static size_t processHttpResponseData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    if (ptr)
    {
        	strncpy((char *)stream, (char *)ptr, size * nmemb);
    }

    return size*nmemb;
}

int httpPostData(const char *url, const char *postdata, int len, char *response)
{
    if (!postdata || !postdata[0] || len < 0 || !response)
    {
        return HTTP_OTHER_ERROR;
    }

    int     ret = HTTP_OK;
    CURL    *curl;
    CURLcode res;
    struct curl_slist *headers=NULL;


    curl = curl_easy_init();
    if(curl)
    {
        // set http head info
        headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf8");
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL,1L);
        curl_easy_setopt(curl, CURLOPT_POST, 1);                // 1: post function 0: get function
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, processHttpResponseData);
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            switch(res)
            {
                case CURLE_UNSUPPORTED_PROTOCOL:
                    ret = HTTP_UNSUPPORTED_PROTOCOL;
                    break;
                case CURLE_COULDNT_CONNECT:
                    ret = HTTP_COULDNT_CONNECT;
                    break;
                case CURLE_HTTP_RETURNED_ERROR:
                    ret = HTTP_HTTP_RETURNED_ERROR;
                    break;
                case CURLE_READ_ERROR:
                    ret = HTTP_READ_ERROR;
                    break;
                default:
                    ret = HTTP_OTHER_ERROR;
                    break;
            }
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
	}
    else
    {
		ret = HTTP_INIT_ERROR;
	}

    return ret;
}



