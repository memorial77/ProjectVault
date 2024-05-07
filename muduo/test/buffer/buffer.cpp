#include "../../src/buffer.hpp"
#include "../../src/log.hpp"

int main()
{
    Buffer buffer;

    // 测试写入字符串
    std::string test_string = "Hello, World!";
    buffer.write_string(test_string);
    if (buffer.readable_size() != test_string.size())
        LOG_MSG(ERROR, "write_string or readable_size failed.");
    else
        LOG_MSG(INFO, "write_string passed.");

    // 测试读取字符串
    std::string read_string = buffer.read_string(test_string.size());
    if (read_string != test_string)
        LOG_MSG(ERROR, "read_string failed.");
    else
        LOG_MSG(INFO, "read_string passed.");

    // 测试写入数据
    char test_data[] = "Hello, GitHub Copilot!";
    buffer.write(test_data, sizeof(test_data));
    if (buffer.readable_size() != sizeof(test_data))
        LOG_MSG(ERROR, "write or readable_size failed.");
    else
        LOG_MSG(INFO, "write passed.");

    // 测试读取数据
    char read_data[sizeof(test_data)];
    buffer.read(read_data, sizeof(test_data));
    if (memcmp(read_data, test_data, sizeof(test_data)) != 0)
        LOG_MSG(ERROR, "read failed.");
    else
        LOG_MSG(INFO, "read passed.");

    // 测试查找CRLF
    buffer.clear();
    buffer.write_string("Hello\nWorld\n");
    char *crlf = buffer.find_crlf();
    if (crlf == nullptr || *crlf != '\n')
        LOG_MSG(ERROR, "find_crlf failed.");
    else
        LOG_MSG(INFO, "find_crlf passed.");

    // 测试读取空Buffer
    std::string empty_read = buffer.read_string(0);
    if (!empty_read.empty())
        LOG_MSG(ERROR, "read_string from empty buffer failed.");
    else
        LOG_MSG(INFO, "read_string from empty buffer passed.");

    buffer.clear();
    // 测试写入空字符串
    buffer.write_string("");
    if (buffer.readable_size() != 0)
        LOG_MSG(ERROR, "write_string with empty string failed.");
    else
        LOG_MSG(INFO, "write_string with empty string passed.");

    // 测试Buffer满时的行为
    char large_data[buffer.writeable_size() + 1];
    memset(large_data, 'a', sizeof(large_data));
    buffer.write(large_data, sizeof(large_data));
    if (buffer.readable_size() != sizeof(large_data))
        LOG_MSG(ERROR, "write with full buffer failed.");
    else
        LOG_MSG(INFO, "write with full buffer passed.");

    // 测试没有CRLF的情况
    buffer.clear();
    buffer.write_string("HelloWorld");
    char *no_crlf = buffer.find_crlf();
    if (no_crlf != nullptr)
        LOG_MSG(ERROR, "find_crlf with no CRLF failed.");
    else
        LOG_MSG(INFO, "find_crlf with no CRLF passed.");

    LOG_MSG(INFO, "Buffer test finished.");
}