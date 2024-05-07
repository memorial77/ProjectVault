#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.hpp"
#include <fcntl.h>

static const int MAX_LISTEN = 1024; // 最大监听数

class Socket
{
public:
    Socket() : sockfd_(-1) {} // 构造函数
    Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket() { Close(); }          // 析构函数
    int GetFd() { return sockfd_; } // 获取文件描述符

    // 创建套接字
    bool Create()
    {
        // AF_INET: IPv4协议 SOCK_STREAM: 流式套接字 IPPROTO_TCP: TCP协议
        sockfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd_ == -1)
        {
            LOG_MSG(ERROR, "create socket failed!");
            return false;
        }

        LOG_MSG(DEBUG, "create socket success!");
        return true;
    }

    // 绑定地址信息
    bool Bind(const std::string &ip, int port)
    {
        struct sockaddr_in addr;
        bzero(&addr, sizeof(addr)); // 清空结构体
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        socklen_t addr_len = sizeof(addr);

        if (bind(sockfd_, (struct sockaddr *)&addr, addr_len) == -1)
        {
            LOG_MSG(ERROR, "bind socket failed!");
            return false;
        }

        LOG_MSG(DEBUG, "bind socket success!");
        return true;
    }

    // 监听套接字
    bool Listen(int backlog = MAX_LISTEN)
    {
        if (listen(sockfd_, backlog) == -1)
        {
            LOG_MSG(ERROR, "listen socket failed!");
            return false;
        }

        LOG_MSG(DEBUG, "listen socket success!");
        return true;
    }

    // 向服务器发起连接
    bool Connect(const std::string &ip, int port)
    {
        struct sockaddr_in server_addr;
        bzero(&server_addr, sizeof(server_addr)); // 清空结构体
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        socklen_t addr_len = sizeof(server_addr);

        if (connect(sockfd_, (struct sockaddr *)&server_addr, addr_len) == -1)
        {
            LOG_MSG(ERROR, "connect socket failed!");
            return false;
        }

        LOG_MSG(DEBUG, "connect socket success!");
        return true;
    }

    // 接受客户端连接
    int Accept()
    {
        struct sockaddr_in client_addr;
        bzero(&client_addr, sizeof(client_addr)); // 清空结构体
        socklen_t addr_len = sizeof(client_addr);

        int client_sockfd = accept(sockfd_, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sockfd == -1)
        {
            LOG_MSG(ERROR, "accept socket failed!");
            return -1;
        }

        LOG_MSG(DEBUG, "accept socket success!");
        return client_sockfd;
    }

    // 接收数据 flag: 0-阻塞接收
    ssize_t Recv(void *buf, size_t len, int flag = 0)
    {
        ssize_t recv_len = recv(sockfd_, buf, len, flag);
        if (recv_len <= 0)
        {
            // 非阻塞模式下，EAGAIN表示没有数据可读，EINTR表示被信号中断
            if (errno == EAGAIN || errno == EINTR)
            {
                LOG_MSG(WARN, "recv data failed!" + std::to_string(errno));
                return 0;
            }
            else
            {
                LOG_MSG(ERROR, "recv data failed!");
                return -1;
            }
        }

        LOG_MSG(DEBUG, "recv data success!");
        return recv_len;
    }

    // 非阻塞接收数据
    ssize_t NonBlockRecv(void *buf, size_t len)
    {
        // MSG_DONTWAIT: 非阻塞接收
        return Recv(buf, len, MSG_DONTWAIT);
    }

    // 发送数据
    ssize_t Send(const void *buf, size_t len, int flag = 0)
    {
        ssize_t send_len = send(sockfd_, buf, len, flag);
        if (send_len < 0)
        {
            LOG_MSG(ERROR, "send data failed!");
            return -1;
        }

        LOG_MSG(DEBUG, "send data success!");
        return send_len;
    }

    // 非阻塞发送数据
    ssize_t NonBlockSend(const void *buf, size_t len)
    {
        // MSG_DONTWAIT: 非阻塞发送
        return Send(buf, len, MSG_DONTWAIT);
    }

    // 获取socket文件描述符
    void Close()
    {
        if (sockfd_ != -1)
        {
            close(sockfd_);
            sockfd_ = -1;
        }
    }

    // 设置非阻塞模式
    void NonBlock()
    {
        int flags = fcntl(sockfd_, F_GETFL, 0); // 获取文件描述符状态标志
        fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK);
        LOG_MSG(DEBUG, "set nonblock success!");
    }

    // 设置地址复用
    void ReuseAddr()
    {
        int opt = 1;
        // SO_REUSEADDR: 允许重用本地地址 SO_REUSEPORT: 允许重用本地端口
        setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        LOG_MSG(DEBUG, "set reuseaddr success!");
    }

    // 创建服务端连接
    bool CreateServer(int port, const std::string &ip = "0.0.0.0")
    {
        if (!Create())
            return false;

        // NonBlock(); // 设置非阻塞模式

        if (!Bind(ip, port))
            return false;

        if (!Listen())
            return false;

        ReuseAddr(); // 设置地址复用

        LOG_MSG(INFO, "create server success!");
        return true;
    }

    // 创建客户端连接
    bool CreateClient(const std::string &ip, int port)
    {
        if (!Create())
            return false;

        NonBlock(); // 设置非阻塞模式

        if (!Connect(ip, port))
            return false;

        LOG_MSG(INFO, "create client success!");
        return true;
    }

private:
    int sockfd_; // socket文件描述符
};