#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include "log.hpp"

int main()
{
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0); // 创建一个定时器
    if(timer_fd == -1)
    {
        log_msg(ERROR, "Failed to create timer");
        return -1;
    }

    // 设置定时器
    struct itimerspec new_value;
    new_value.it_value.tv_sec = 3; // 第一次超时时间
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 1; // 之后每次超时时间
    new_value.it_interval.tv_nsec = 0;
    timerfd_settime(timer_fd, 0, &new_value, NULL);

    while(1)
    {
        u_int64_t exp;
        read(timer_fd, &exp, sizeof(exp)); // 读取定时器超时次数
        // printf("Timer expired %llu times\n", exp);
        log_msg(INFO, "Timer expired " + std::to_string(exp) + " times");
    }
}
