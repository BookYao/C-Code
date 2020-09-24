#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rabbitmq.h"

//#include "utils.h"


int main(int argc, char const *const *argv) {
    T_MQ_Conf_Info confInfo;
    memset(&confInfo, 0, sizeof(T_MQ_Conf_Info));

    T_MQ_Conn_Info *connInfo;

    char buf[128] = {'\0'};

    if (argc < 4) {
        fprintf(
                stderr,
                "Usage: amqp_sendstring host port messagebody\n");
        return 1;
    }

    strncpy(buf, argv[3], sizeof(buf) - 1);

    strncpy(confInfo.host, argv[1], sizeof(confInfo.host) - 1);
    confInfo.port = atoi(argv[2]);
    strncpy(confInfo.rmquser, "baseuser", sizeof(confInfo.rmquser) - 1);
    strncpy(confInfo.rmqpasswd, "basepasswd", sizeof(confInfo.rmqpasswd) - 1);
    strncpy(confInfo.vhost, "vhostBasemq", sizeof(confInfo.vhost) - 1);
    confInfo.channelid = 1;

    connInfo = initMQConn(&confInfo);
    setExchange(connInfo, "amq.fanout", "fanout");
    setRouteKey(connInfo, "baseKey");

    publishMsg(connInfo, buf);

    sleep(10);
    
    destoryMQConn(connInfo);
    return 0;
}


