

#include <stdio.h>
#include <unistd.h>

#include "./include/syscfg.h"


int main(void)
{
    char ip[32] = {0};
    char port[32] = {0};

    while(1)
    {
        sysconfReset();
        if (sysconfGet("OPT", "opt_servaddr", ip)  == NO_ERROR)
        {
            printf("ip:%s\n", ip);
        }

        if (sysconfGet("OPT", "opt_servport", port) == NO_ERROR)
        {
            printf("port:%s\n", port);
        }
        sleep(3);
    }
    return 0;
}
