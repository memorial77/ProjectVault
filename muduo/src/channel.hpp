#include <functional>
#include <sys/epoll.h>
#include "log.hpp"

using event_callback = std::function<void()>; // 事件回调函数

class Channel
{
public:
    Channel(int fd) : fd_(fd), events_(0), revents_(0) {} //  构造函数
    int fd() const { return fd_; }                        // 获取文件描述符

    void set_revents(uint32_t revents) { revents_ = revents; } // 设置触发事件

    void set_read_callback(const event_callback &cb) { read_callback_ = cb; }   // 设置读事件回调函数
    void set_write_callback(const event_callback &cb) { write_callback_ = cb; } // 设置写事件回调函数
    void set_error_callback(const event_callback &cb) { error_callback_ = cb; } // 设置错误事件回调函数
    void set_close_callback(const event_callback &cb) { close_callback_ = cb; } // 设置关闭事件回调函数
    void set_event_callback(const event_callback &cb) { event_callback_ = cb; } // 设置任意事件回调函数

    bool read_enabled() const { return events_ & EPOLLIN; }   // 读事件是否开启
    bool write_enabled() const { return events_ & EPOLLOUT; } // 写事件是否开启

    void enable_read() { events_ |= EPOLLIN; }   // 开启读事件
    void enable_write() { events_ |= EPOLLOUT; } // 开启写事件

    void disable_read() { events_ &= ~EPOLLIN; }   // 关闭读事件
    void disable_write() { events_ &= ~EPOLLOUT; } // 关闭写事件
    void disable_all() { events_ = 0; }            // 关闭所有事件

    // 处理事件
    void handle_event()
    {
        // EPOLLIN: 有数据可读 | EPOLLRDHUP: 对端关闭连接 | EPOLLPRI: 有紧急数据可读
        if ((revents_ & EPOLLIN) || (revents_ & EPOLLRDHUP) || (revents_ & EPOLLPRI))
        {
            if (read_callback_)
                read_callback_();

            // 任意事件
            if (event_callback_)
                event_callback_();
        }

        if (revents_ & EPOLLOUT) // EPOLLOUT: 可写
        {

            if (write_callback_)
                write_callback_();

            // 事件处理完毕后调用任意事件回调函数 刷新活跃度
            if (event_callback_) 
                event_callback_();
        }
        else if (revents_ & EPOLLERR) // EPOLLERR: 错误
        {
            // 一旦出错则释放连接 不调用任意事件的回调函数
            if (error_callback_)
                error_callback_();
        }
        else if (revents_ & EPOLLHUP) // EPOLLHUP: 对端关闭连接
        {
            if (close_callback_)
                close_callback_();
        }
    }

private:
    int fd_;           // 监控的文件描述符
    uint32_t events_;  // 当前监控事件
    uint32_t revents_; // 当前连接触发事件

    event_callback read_callback_;  // 读事件回调函数
    event_callback write_callback_; // 写事件回调函数
    event_callback error_callback_; // 错误事件回调函数
    event_callback close_callback_; // 关闭事件回调函数
    event_callback event_callback_; // 任意事件回调函数
};