
#ifndef __HTTP_INTERFACE_H_
#define __HTTP_INTERFACE_H_

typedef enum httpErrorCode
{
    HTTP_OK,
    HTTP_UNSUPPORTED_PROTOCOL,
    HTTP_COULDNT_CONNECT,
    HTTP_HTTP_RETURNED_ERROR,
    HTTP_READ_ERROR,
    HTTP_INIT_ERROR,
    HTTP_OTHER_ERROR
} HttpErrorCode;

int httpPostData(const char *url, const char *postdata, int len, char *response);

char *getHttpErrorCode(int error);


#endif
