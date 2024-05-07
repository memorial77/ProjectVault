#pragma once

#include <stdlib.h>
#include <vector>
#include <string>
#include <string.h>
#include <cassert>
#include "log.hpp"

static const size_t BUFFER_DEAULT_SIZE = 1024; // 初始缓冲区大小

class Buffer
{
public:
    // 构造函数
    Buffer(size_t init_size = BUFFER_DEAULT_SIZE) : read_index_(0), write_index_(0), buffer_(init_size) {}

    // 获取写入地址
    char *begin_write() { return &buffer_[write_index_]; }

    // 获取读取地址
    char *begin_read() { return &buffer_[read_index_]; }

    // 获取前沿空闲空间大小
    size_t head_free_size() const { return read_index_; }

    // 获取后沿空闲空间大小
    size_t back_free_size() const { return buffer_.size() - write_index_; }

    // 获取可读数据大小
    size_t readable_size() const { return write_index_ - read_index_; }

    // 获取可写数据大小
    size_t writeable_size() const { return head_free_size() + back_free_size(); }

    // 读索引前移
    void move_read_off(size_t len)
    {
        assert(len <= readable_size());
        read_index_ += len;
    }

    // 写索引前移
    void move_write_off(size_t len)
    {
        assert(len <= back_free_size());
        write_index_ += len;
    }

    // 确保可写空间足够
    void ensure_writeable(size_t len)
    {
        if (back_free_size() >= len)
            // 后沿空闲空间足够直接返回
            return;
        else if (writeable_size() >= len)
        {
            // 后沿空闲空间不够，但前后空闲空间总和足够
            size_t readable = readable_size();                   // 可读数据大小
            std::copy(begin_read(), begin_write(), &buffer_[0]); // 将数据移到前端
            read_index_ = 0;                                     // 重置读索引
            write_index_ = read_index_ + readable;               // 重置写索引
        }
        else
        {
            // 前后空闲空间总和不够
            buffer_.resize(write_index_ + len); // 扩容
        }
    }

    // 写入数据
    void write(const char *data, size_t len)
    {
        ensure_writeable(len);                      // 确保可写空间足够
        std::copy(data, data + len, begin_write()); // 写入数据
        move_write_off(len);                        // 写索引前移
    }

    // 写入字符串
    void write_string(const std::string &data) { write(data.c_str(), data.size()); }

    // 写入缓冲区
    void write_buffer(Buffer &buffer) { write(buffer.begin_read(), buffer.readable_size()); }

    // 读取数据
    void read(char *data, size_t len)
    {
        assert(len <= readable_size());                    // 确保可读数据足够
        std::copy(begin_read(), begin_read() + len, data); // 读取数据
        move_read_off(len);                                // 读索引前移
    }

    // 读取字符串
    std::string read_string(size_t len)
    {
        assert(len <= readable_size()); // 确保可读数据足够
        std::string data(begin_read(), begin_read() + len);
        move_read_off(len); // 读索引前移
        return data;
    }

    // 查找CRLF
    char *find_crlf()
    {
        void *ret = memchr(begin_read(), '\n', readable_size()); // 查找换行符
        return static_cast<char *>(ret);                         // 返回查找结果
    }

    // 读取行数据
    std::string read_line()
    {
        char *crlf = find_crlf(); // 查找换行符
        if (crlf)
            return read_string(crlf - begin_read() + 1); // 读取一行数据 +1是为了包含换行符

        return ""; // 未找到换行符返回空字符串
    }

    // 清空缓冲区
    void clear()
    {
        read_index_ = 0;
        write_index_ = 0;
    }

private:
    size_t read_index_;        // 读索引
    size_t write_index_;       // 写索引
    std::vector<char> buffer_; // 缓冲区
};