#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rabbitmq.h"

//#include "utils.h"


/* TestDemo: ./rabbitSenc 127.0.0.1 5672 */
int main(int argc, char const *const *argv) {
    T_MQ_Conf_Info confInfo;
    memset(&confInfo, 0, sizeof(T_MQ_Conf_Info));

    T_MQ_Conn_Info *connInfo;

    char buf[128] = {'\0'};

    if (argc < 3) {
        fprintf(
                stderr,
                "Usage: amqp_sendstring host port\n");
        return 1;
    }

    //strncpy(buf, argv[3], sizeof(buf) - 1);

    strncpy(confInfo.host, argv[1], sizeof(confInfo.host) - 1);
    confInfo.port = atoi(argv[2]);
    strncpy(confInfo.rmquser, "rcspuser", sizeof(confInfo.rmquser) - 1);
    strncpy(confInfo.rmqpasswd, "rcsppasswd", sizeof(confInfo.rmqpasswd) - 1);
    strncpy(confInfo.vhost, "rcsp_vhost", sizeof(confInfo.vhost) - 1);
    confInfo.channelid = 1;

    connInfo = initMQConn(&confInfo);
	if (!connInfo) {
		printf("MQ conninfo failed\n");
		return -1;
	}

	//设置交换机groupExc, topic 模式
    setExchange(connInfo, "groupExc", "topic");

	//routintKey 是 group.# 可接收消息
    setRouteKey(connInfo, "group");

	snprintf(buf, sizeof(buf) - 1, "%s", "{\"corp\":\"1000\",\"grpNum\":\"2000\"}");
	printf("start Group publish msg\n");
    publishMsg(connInfo, buf);

    sleep(3);

	//设置交换机groupExc, topic 模式
    setExchange(connInfo, "groupExc", "topic");

	//routingKey is contacts.# 的可以接收消息
    setRouteKey(connInfo, "contacts");

	printf("start Contact publish msg\n");
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%s", "{\"corp\":\"1000\", \"version\":1,\"fileName\":\"test.txt\", \"dir\":\"root\"}");
    publishMsg(connInfo, buf);
    
    destoryMQConn(connInfo);
    return 0;
}


