

#ifndef __MSG_PACKET_H
#define __MSG_PACKET_H


#define CONTEXT_LENGTH      128
#define URL_LEN             256


typedef struct _MsgText {
    char    name[CONTEXT_LENGTH];
    char    identity[CONTEXT_LENGTH];
    char    timeout[CONTEXT_LENGTH];
} T_MsgText;

typedef struct _HttpResMsg {
    char    code[CONTEXT_LENGTH];
    char    message[CONTEXT_LENGTH];
    char    success[CONTEXT_LENGTH];
    char    data[CONTEXT_LENGTH];
} T_HttpResMsg;

int msgTextParse(char *msgText, T_MsgText *msgJson);
int buildSignUpOfflineMsg(T_MsgText *msgText, char *signupOfflineMsg, int len);
int parseHttpResMsg(char *msgText, T_HttpResMsg *resMsg);



#endif
