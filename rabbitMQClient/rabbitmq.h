
#ifndef __RABBIT_MQ_H
#define __RABBIT_MQ_H

#include "amqp.h"

#define RMQ_CONF_LEN 32 

typedef struct mqConfInfo {
    char    host[17];
    int     port;
    char    rmquser[RMQ_CONF_LEN];
    char    rmqpasswd[RMQ_CONF_LEN];

    int     channelid;
    char    vhost[RMQ_CONF_LEN];
    char    exchange[RMQ_CONF_LEN];
    char    exchangeType[RMQ_CONF_LEN];
    char    routeKey[RMQ_CONF_LEN];
} T_MQ_Conf_Info;

typedef struct mqConnInfo {
    amqp_connection_state_t     conn;
    int                         channelid;
    char                        vhost[RMQ_CONF_LEN];
    char                        exchange[RMQ_CONF_LEN];
    char                        exchangeType[RMQ_CONF_LEN];
    char                        routeKey[RMQ_CONF_LEN];
    pthread_mutex_t             mqLock;
} T_MQ_Conn_Info;


T_MQ_Conn_Info *initMQConn(T_MQ_Conf_Info *confInfo);
void destoryMQConn(T_MQ_Conn_Info *mqConn);

int publishMsg(T_MQ_Conn_Info *mqConnInfo, char *msg);


void setExchange(T_MQ_Conn_Info *mqConnInfo, char *exchange, char *exchangeType);
void setRouteKey(T_MQ_Conn_Info *mqConnInfo, char *routeKey);
int exchangeDeclare(T_MQ_Conn_Info *mqConnInfo);
int declareQueue(T_MQ_Conn_Info *mqConnInfo, char const *queue);
int declareQueueAndBindQueue(T_MQ_Conn_Info *mqConnInfo, char const *queue);
int recvMsgFromMQ(T_MQ_Conn_Info *mqConnInfo, char **msg);




#endif
