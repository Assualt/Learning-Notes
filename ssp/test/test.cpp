#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

void PrintProcess(time_t now, uint32_t total, uint32_t cur, uint32_t speed, uint32_t left)
{
    struct tm *t = localtime(&now);
    printf("[");
    for (uint32_t i = 0; i < total; ++i) {
        if (i < cur) {
            printf("=");
        } else if (i == cur) {
            printf(">");
        } else {
            printf(" ");
        }
    }

    printf("] %u KB/s,Left %u %4u/%02u/%02u %02u:%02u:%02u\n", speed, left, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    if (cur != 0 || cur < total - 1) {
        printf("\033[A");
        printf("\033[K");
    }
}

int main(int argc, char **argv)
{
    uint32_t idx = 0;
    uint32_t leftTimes = 100;
    uint32_t total = 100;
    while (1) {
        time_t now = time(NULL);


        PrintProcess(now, total, idx, 100, leftTimes--);
        if (idx >= total) {
            printf("finished\n");
            break;   
        }
        idx++;
        usleep(1000 * 50);
    }

    return 0;
}