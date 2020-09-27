
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include "logger.h"
#include "rabbitmq.h"
#include "amqp_tcp_socket.h"

void mqConnInfoBuild(T_MQ_Conn_Info *mqConnInfo, T_MQ_Conf_Info *confInfo)
{
    mqConnInfo->channelid = confInfo->channelid;
    strncpy(mqConnInfo->vhost, confInfo->vhost, sizeof(mqConnInfo->vhost) - 1);
}

T_MQ_Conn_Info *initMQConn(T_MQ_Conf_Info *confInfo)
{
    if (!confInfo) {
        //PrintMsgTrace("%s.%d. initMQConn Param error!", __FUNCTION__, __LINE__);
        return NULL;
    }

    amqp_socket_t *socket = NULL;
    amqp_rpc_reply_t x;
    int status;

    T_MQ_Conn_Info *mqConnInfo = (T_MQ_Conn_Info *)malloc(sizeof(T_MQ_Conn_Info));
    if (!mqConnInfo) {
        //PrintMsgTrace("%s.%d. initMQConn Malloc Failed!", __FUNCTION__, __LINE__);
        return NULL;
    }

    memset(mqConnInfo, 0, sizeof(T_MQ_Conn_Info));
    mqConnInfo->conn = amqp_new_connection();
    if (mqConnInfo->conn == NULL)
    {
        //PrintMsgTrace("%s.%d. amqp_new_connection Failed", __FUNCTION__, __LINE__);
        free(mqConnInfo);
        return NULL;
    }

    mqConnInfoBuild(mqConnInfo, confInfo);
    
    socket = amqp_tcp_socket_new(mqConnInfo->conn);
    if (!socket) {
        //PrintMsgTrace("%s.%d. amqp_tcp_socket_new failed!", __FUNCTION__, __LINE__);
        amqp_destroy_connection(mqConnInfo->conn);
        free(mqConnInfo);
        return NULL;
    }
    
    status = amqp_socket_open(socket, confInfo->host, confInfo->port);
    if (status) {
        //PrintMsgTrace("%s.%d. amqp_socket_open failed!", __FUNCTION__, __LINE__);
        amqp_connection_close(mqConnInfo->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(mqConnInfo->conn);
        free(mqConnInfo);
        return NULL;
    }
    
    amqp_login(mqConnInfo->conn, confInfo->vhost, 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, 
                confInfo->rmquser, confInfo->rmqpasswd);
                
    amqp_channel_open(mqConnInfo->conn, mqConnInfo->channelid);
    x = amqp_get_rpc_reply(mqConnInfo->conn);
    if (x.reply_type != AMQP_RESPONSE_NORMAL) {
        amqp_channel_close(mqConnInfo->conn, confInfo->channelid, AMQP_REPLY_SUCCESS);
        amqp_connection_close(mqConnInfo->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(mqConnInfo->conn);
        free(mqConnInfo);
        return NULL;
    }

    return mqConnInfo;
}


void destoryMQConn(T_MQ_Conn_Info *mqConnInfo)
{
    if (mqConnInfo) {
        amqp_channel_close(mqConnInfo->conn, mqConnInfo->channelid, AMQP_REPLY_SUCCESS);
        amqp_connection_close(mqConnInfo->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(mqConnInfo->conn);
        free(mqConnInfo);
    }
}



int publishMsg(T_MQ_Conn_Info *mqConnInfo, char *msg)
{
    if (!mqConnInfo || !msg || !msg[0]) {
        //PrintMsgTrace("%s.%d. publishMsg Param error!", __FUNCTION__, __LINE__);
        return -1;
    }
    
    amqp_basic_properties_t props;
    amqp_rpc_reply_t x;
    props._flags = AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.delivery_mode = 2; /* persistent delivery mode */
    props.expiration = amqp_cstring_bytes("5");
    
    if (mqConnInfo->conn) {
        amqp_basic_publish(mqConnInfo->conn,
                mqConnInfo->channelid,
                amqp_cstring_bytes(mqConnInfo->exchange),
                amqp_cstring_bytes(mqConnInfo->routeKey),
                0,
                0,
                &props,
                amqp_cstring_bytes(msg));
        x = amqp_get_rpc_reply(mqConnInfo->conn);
        if (x.reply_type != AMQP_RESPONSE_NORMAL) {
            //PrintMsgTrace("%s.%d. publishMsg Param error!", __FUNCTION__, __LINE__);
            return -1;
        }
    } 
    
    return 0;
}

void setExchange(T_MQ_Conn_Info *mqConnInfo, char *exchange, char *exchangeType)
{
    if (!mqConnInfo) {
        //PrintMsgTrace("%s.%d. setExchange Param error!", __FUNCTION__, __LINE__);
        return ;
    }

    strncpy(mqConnInfo->exchange, exchange, sizeof(mqConnInfo->exchange) - 1);
    strncpy(mqConnInfo->exchangeType, exchangeType, sizeof(mqConnInfo->exchangeType) - 1);     
}

void setRouteKey(T_MQ_Conn_Info *mqConnInfo, char *routeKey)
{
    if (!mqConnInfo) {
        //PrintMsgTrace("%s.%d. setExchange Param error!", __FUNCTION__, __LINE__);
        return ;
    }
    strncpy(mqConnInfo->routeKey, routeKey, sizeof(mqConnInfo->routeKey) - 1);   
}

int exchangeDeclare(T_MQ_Conn_Info *mqConnInfo)
{
    if (mqConnInfo == NULL) {
        //PrintMsgTrace("%s.%d. exchangeDeclare Param error!", __FUNCTION__, __LINE__);
        return -1;
    }

    amqp_rpc_reply_t x;
    amqp_exchange_declare(mqConnInfo->conn, mqConnInfo->channelid, 
                            amqp_cstring_bytes(mqConnInfo->exchange),
                            amqp_cstring_bytes(mqConnInfo->exchangeType), 
                            0, 0, 0, 0, amqp_empty_table);
    x = amqp_get_rpc_reply(mqConnInfo->conn);
    if (x.reply_type != AMQP_RESPONSE_NORMAL ) {
        //PrintMsgTrace("%s.%d. amqp_exchange_declare failed!", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

/************************************
 *
 *queue :Qxxx(sip number)
 *exchange
 *routingkey :xxx(sip number)
 * 
 ***********************************/
int declareQueue(T_MQ_Conn_Info *mqConnInfo, char const *queue)
{
    if (mqConnInfo == NULL) {
        //PrintMsgTrace("%s.%d. declareQueue Param error!", __FUNCTION__, __LINE__);
        return -1;
    }
    
    amqp_rpc_reply_t x;
    amqp_queue_declare_ok_t *r = amqp_queue_declare(mqConnInfo->conn, 
                                                    mqConnInfo->channelid, 
                                                    amqp_cstring_bytes(queue), 
                                                    0, 1, 0, 0, amqp_empty_table);
    x = amqp_get_rpc_reply(mqConnInfo->conn);
    if (x.reply_type != AMQP_RESPONSE_NORMAL) {
        //PrintMsgTrace("%s.%d. amqp_queue_declare failed!", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

int declareQueueAndBindQueue(T_MQ_Conn_Info *mqConnInfo, char const *queue)
{
    if (mqConnInfo == NULL) {
        //PrintMsgTrace("%s.%d. declareQueueAndBindQueue Param error!", __FUNCTION__, __LINE__);
        return -1;
    }

    amqp_rpc_reply_t x;
    amqp_queue_declare_ok_t *r = amqp_queue_declare(mqConnInfo->conn, 
                                                    mqConnInfo->channelid, 
                                                    amqp_cstring_bytes(queue),
                                                    0, 1, 0, 0, amqp_empty_table);
    x = amqp_get_rpc_reply(mqConnInfo->conn);
    if (x.reply_type != AMQP_RESPONSE_NORMAL) {
        //PrintMsgTrace("%s.%d. amqp_queue_declare failed!", __FUNCTION__, __LINE__);
        return -1;
    }
    
    amqp_queue_bind(mqConnInfo->conn, mqConnInfo->channelid, amqp_cstring_bytes(queue), 
                    amqp_cstring_bytes(mqConnInfo->exchange),
                    amqp_cstring_bytes(mqConnInfo->routeKey), amqp_empty_table);
    if (x.reply_type != AMQP_RESPONSE_NORMAL) {
        //PrintMsgTrace("%s.%d. amqp_queue_bind failed!", __FUNCTION__, __LINE__);
        return -1;
    }
    
    amqp_basic_consume(mqConnInfo->conn, mqConnInfo->channelid,  amqp_cstring_bytes(queue), 
                        amqp_empty_bytes, 0, 1, 0, amqp_empty_table); //use no_ack mode.
    if (x.reply_type != AMQP_RESPONSE_NORMAL) {
        //PrintMsgTrace("%s.%d. amqp_basic_consume failed!", __FUNCTION__, __LINE__);
        return -1;
    }
    
    return 0;
}

int recvMsgFromMQ(T_MQ_Conn_Info *mqConnInfo, char **msg)
{
    amqp_rpc_reply_t res;
    amqp_envelope_t envelope;

    /**
     * 释放amqp_connection_state_t占用的内存
     *
     * 释放与任何通道相关的amqp_connection_state_t对象拥有的内存，允许库重用。
     * 在调用该函数之前使用库返回的任何内存，会导致未定义的行为。
     */
    amqp_maybe_release_buffers(mqConnInfo->conn);

    /**
     * 等待并消费一条消息
     *
     * 在任何频道上等待basic.deliver方法，一旦收到basic.deliver它读取该消息，并返回。
     * 如果在basic.deliver之前接收到任何其他方法，则此函数将返回一个包含
     * ret.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION和
     * ret.library_error == AMQP_STATUS_UNEXPECTED_STATE的amqp_rpc_reply_t。
     * 然后调用者应该调用amqp_simple_wait_frame()来读取这个帧并采取适当的行动。
     *
     * 在使用amqp_basic_consume()函数启动消费者之后，应该使用此函数
     *
     *  \returns 一个amqp_rpc_reply_t对象，成功时，ret.reply_type == AMQP_RESPONSE_NORMAL
     *  如果ret.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION，并且
     *  ret.library_error == AMQP_STATUS_UNEXPECTED_STATE
     *  如果收到AMQP_BASIC_DELIVER_METHOD以外的帧，则调用者应调用amqp_simple_wait_frame()
     *  来读取此帧并采取适当的操作。
     */
    res = amqp_consume_message(mqConnInfo->conn, &envelope, NULL, 0);
    if (AMQP_RESPONSE_NORMAL != res.reply_type) {
        //PrintMsgTrace("%s.%d. amqp_consume_message failed!", __FUNCTION__, __LINE__);
        return -1;
    }

    printf("Delivery %u, exchange %.*s routingkey %.*s\n",
            (unsigned)envelope.delivery_tag, (int)envelope.exchange.len,
            (char *)envelope.exchange.bytes, (int)envelope.routing_key.len,
            (char *)envelope.routing_key.bytes);

    if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        printf("Content-type: %.*s\n",
                (int)envelope.message.properties.content_type.len,
                (char *)envelope.message.properties.content_type.bytes);
    }

    printf("consume msg: %s\n", (char *)(envelope.message.body.bytes));

    int msgLen = envelope.message.body.len;
    int buflen = msgLen + 1;
    char *msgPtr = (char *)malloc(buflen * sizeof(char));
    if (msgPtr) {
        memset(msgPtr, 0, buflen);
        memcpy(msgPtr, (char *)(envelope.message.body.bytes), msgLen);
        *msg = msgPtr;
    }
    

    //amqp_dump(envelope.message.body.bytes, envelope.message.body.len);

    /**
     * 释放在amqp_consume_message()中分配的与amqp_envelope_t相关联的内存
     */
    amqp_destroy_envelope(&envelope);

    return msgLen;
}



