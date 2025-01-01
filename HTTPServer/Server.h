#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <string>
#include <mutex>

// Visualization of socket connections
// 1. Listening Socket
// Server:  "I'm waiting for someone to knock on port 8080."
// Clients: "I'm trying to connect to port 8080."
// 2. Dedicated Socket (Per Client):
// Server: "Welcome! Here's your private room to chat with me."
// Client: "Great! Let's start our conversation."

class Server {
public:
    Server(int port); // constructor declaration
    ~Server(); // destructor declaration
    bool initialize();
    bool bindSocket();
    bool startListening(int backlog = 5);
    void handleClient(SOCKET clientSocket, std::string clientIP, unsigned short clientPort);
    void acceptClient();
    void handleRequest(SOCKET clientSocket);
    void handleGetRequest(SOCKET clientSocket, const std::string& path);
    void handlePostRequest(SOCKET clientSocket, std::istringstream& requestStream, const char* requestData);

private:
    int port;
    SOCKET serverSocket;
    sockaddr_in serverAddr;
};

#endif // SERVER_H
