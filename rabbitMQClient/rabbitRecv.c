
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rabbitmq.h"


/* Usage demo: ./rabbitRecv 127.0.0.1 5672 groupQueue */
int main(int argc, char const *const *argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: amqp_listen host port Queue\n");
        return 1;
    }

    T_MQ_Conf_Info confInfo;
    T_MQ_Conn_Info *connInfo;
    memset(&confInfo, 0, sizeof(T_MQ_Conf_Info));


    strncpy(confInfo.host, argv[1], sizeof(confInfo.host) - 1);
    confInfo.port = atoi(argv[2]);
    strncpy(confInfo.rmquser, "rcspuser", sizeof(confInfo.rmquser) - 1);
    strncpy(confInfo.rmqpasswd, "rcsppasswd", sizeof(confInfo.rmqpasswd) - 1);

    strncpy(confInfo.vhost, "rcsp_vhost", sizeof(confInfo.vhost) - 1);
    confInfo.channelid = 1;

    connInfo = initMQConn(&confInfo);
	if (!connInfo) {
		printf("MQ conn failed.\n");
		return -1;
	}
    
    //setExchange(connInfo, "amq.fanout", "fanout");
    setExchange(connInfo, "groupExc", "topic");
    setRouteKey(connInfo, "group.#");   
    declareQueue(connInfo, argv[3]);
    declareQueueAndBindQueue(connInfo, argv[3]);

    int msgLen = 0;
    char *msg = NULL;

#if 1
    for (;;) {
        msgLen = recvMsgFromMQ(connInfo, &msg);
        printf("msg:%s-len:%d\n", msg, msgLen);
        free(msg);
    }
#endif

    destoryMQConn(connInfo);

    return 0;
}



