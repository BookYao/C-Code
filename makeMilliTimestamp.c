

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

int makeMilliTimestamp(char *timestamp, int buflen) {
    if (!timestamp || buflen < 0) {
        printf("makeMilliTimestamp param error!\n");
        return -1;
    }

    struct timeval tv;
    memset(&tv, '\0', sizeof(struct timeval));

    gettimeofday(&tv, NULL);
    snprintf(timestamp, buflen - 1, "%ld%ld", tv.tv_sec, tv.tv_usec/1000);
    return 0;
}

int main() {
    char    milliTimestamp[16] = {'\0'};

    makeMilliTimestamp(milliTimestamp, sizeof(milliTimestamp));
    printf("milliTimestamp:%s\n", milliTimestamp);
    
    return 0;
}
