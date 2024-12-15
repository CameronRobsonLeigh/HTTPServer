#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <string>

class Server {
public:
    Server(int port);
    ~Server();

    bool initialize();
    bool bindSocket();
    bool startListening(int backlog = 5);
    void acceptClient();
    void handleRequest(SOCKET clientSocket);

private:
    int port;
    SOCKET serverSocket;
    sockaddr_in serverAddr;
};

#endif // SERVER_H
