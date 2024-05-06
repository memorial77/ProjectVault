#include <sys/timerfd.h>
#include <unordered_map>
#include <functional>
#include <stdio.h>
#include <memory>
#include <vector>
#include "log.hpp"
#include <unistd.h>

using task_func = std::function<void()>;    // 定时器任务回调函数
using release_func = std::function<void()>; // 定时器任务释放函数

static const int DEFAULT_TICK = 0;        // 默认时间轮刻度
static const int DEFAULT_WHEEL_SIZE = 60; // 默认时间轮容量

// 定时器任务类
class TimerTask
{
public:
    TimerTask(uint64_t id, uint64_t interval, task_func task)
        : id_(id), interval_(interval), task_(task), valid_(true) {}

    ~TimerTask()
    {
        if (valid_)
            task_(); // 执行定时器任务

        release_(); // 释放定时器任务
    }

    // 设置定时器任务释放函数
    void set_release(const release_func &release) { release_ = release; }

    // 获取定时器任务时间间隔
    uint64_t interval() const { return interval_; }

    // 设置定时器任务无效
    void set_invalid() { valid_ = false; }

private:
    uint64_t id_;          // 定时器任务id
    uint64_t interval_;    // 定时器任务间隔
    task_func task_;       // 定时器任务回调函数
    release_func release_; // 定时器任务释放函数
    bool valid_;           // 定时器任务是否有效
};

class TimerWheel
{
public:
    TimerWheel(int capacity = DEFAULT_WHEEL_SIZE, int tick = DEFAULT_TICK)
        : tick_(tick), capacity_(capacity), wheel_(capacity) {}

    void timer_add(uint64_t id, uint64_t interval, task_func task)
    {
        // 无效的定时器间隔
        if (interval <= 0)
            return;

        // 创建定时器任务
        auto timer = std::make_shared<TimerTask>(id, interval, task);
        timer->set_release(std::bind(&TimerWheel::remove_timer, this, id));

        // 计算定时器任务位置
        int pos = (tick_ + interval) % capacity_;
        // 添加定时器任务
        wheel_[pos].push_back(timer);
        timers_[id] = std::weak_ptr<TimerTask>(timer);
    }

    void remove_timer(uint64_t id)
    {
        auto it = timers_.find(id);
        if (it == timers_.end())
            return;

        timers_.erase(it); // 移除定时器任务
    }

    void refresh_timer(uint64_t id)
    {
        auto it = timers_.find(id);
        // 定时器任务不存在
        if (it == timers_.end())
            return;

        std::shared_ptr<TimerTask> timer = it->second.lock(); // 获取定时器任务
        uint64_t interval = timer->interval();                // 获取定时器任务时间间隔
        int pos = (tick_ + interval) % capacity_;             // 计算定时器任务位置
        wheel_[pos].push_back(timer);                         // 添加定时器任务
    }

    void run_timer_task()
    {
        tick_ = (tick_ + 1) % capacity_; // 更新时间轮刻度
        wheel_[tick_].clear();           // 清空时间轮刻度
    }

    void cancel_timer(uint64_t id)
    {
        auto it = timers_.find(id);
        // 定时器任务不存在
        if (it == timers_.end())
            return;

        std::shared_ptr<TimerTask> timer = it->second.lock(); // 获取定时器任务
        timer->set_invalid();                                 // 设置定时器任务无效
    }

private:
    int tick_;     // 时间轮刻度
    int capacity_; // 时间轮容量

    std::vector<std::vector<std::shared_ptr<TimerTask>>> wheel_;    // 时间轮
    std::unordered_map<uint64_t, std::weak_ptr<TimerTask>> timers_; // 定时器任务
};

int main()
{
    // 创建时间轮
    TimerWheel timer_wheel;
    // 添加定时器任务
    timer_wheel.timer_add(1, 5, []()
                          { log_msg(INFO, "timer 1"); });
    timer_wheel.timer_add(2, 2, []()
                          { log_msg(INFO, "timer 2"); });
    timer_wheel.timer_add(3, 15, []()
                          { log_msg(INFO, "timer 3"); });

    while (true)
    {
        sleep(1);
        log_msg(INFO, "tick");
        timer_wheel.run_timer_task(); // 运行定时器任务
    }

    return 0;
}
