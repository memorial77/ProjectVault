#include "../../src/sock.hpp"
#include "../../src/log.hpp"

int main()
{
    Socket client_socket;
    client_socket.Create();

    client_socket.Connect("127.0.0.1", 8888);

    while(true)
    {
        std::string data;
        std::cin >> data;

        client_socket.Send(data.c_str(), data.size());

        char buf[1024] = {0};
        client_socket.Recv(buf, sizeof(buf));

        LOG_MSG(DEBUG, "recv data: " + std::string(buf));
    }
}