#include "Server.h"
#include <iostream>
#include <WS2tcpip.h>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

Server::Server(int port) : port(port), serverSocket(INVALID_SOCKET) {
    ZeroMemory(&serverAddr, sizeof(serverAddr));
}

Server::~Server() {
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
    }
    WSACleanup();
}

bool Server::initialize() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    return true;
}

bool Server::bindSocket() {
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        return false;
    }
    return true;
}

bool Server::startListening(int backlog) {
    if (listen(serverSocket, backlog) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        return false;
    }
    std::cout << "Server is listening on port " << port << "..." << std::endl;
    return true;
}

void Server::acceptClient() {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed" << std::endl;
        return;
    }

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    unsigned short clientPort = ntohs(clientAddr.sin_port);

    std::cout << "Client connected: " << clientIP << ":" << clientPort << std::endl;

    handleRequest(clientSocket);
    closesocket(clientSocket);
}

void Server::handleRequest(SOCKET clientSocket) {
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Received request:\n" << buffer << std::endl;

        std::istringstream requestStream(buffer);
        std::string method, path, version;
        requestStream >> method >> path >> version;

        std::cout << "HTTP Method: " << method << std::endl;
        std::cout << "Requested Path: " << path << std::endl;

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, world!";
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else {
        std::cerr << "Failed to receive data from client" << std::endl;
    }
}
