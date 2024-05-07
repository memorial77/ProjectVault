#include "../../src/sock.hpp"
#include "../../src/log.hpp"

int main()
{
    set_log_level(DEBUG); // 设置日志级别

    Socket listen_socket;
    listen_socket.CreateServer(8888); // 创建服务器

    while(true)
    {
        Socket client_socket = listen_socket.Accept(); // 接受客户端连接

        while(true)
        {
            char buf[1024] = {0};
            int ret = client_socket.Recv(buf, sizeof(buf)); // 接收数据
            if (ret <= 0)   
                break;

            LOG_MSG(DEBUG, "recv data: " + std::string(buf));

            client_socket.Send(buf, strlen(buf)); // 发送数据

            if (strcmp(buf, "exit") == 0)
                break;
        }
        close(client_socket.GetFd()); // 关闭客户端连接
        LOG_MSG(DEBUG, "client close connection!");
    } 
    
    
}