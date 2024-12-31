#include "Server.h"
#include <iostream>
#include <WS2tcpip.h>
#include <sstream>
#include <map>
#include <algorithm>
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

// the backlog is a "waiting room" for clients, if the server is busy processing, the new client waits in the room.
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

        if (method == "POST") {
            handlePostRequest(clientSocket, requestStream, buffer + bytesReceived);
        }
        else if (method == "GET") {
            handleGetRequest(clientSocket, path);
        }
        else {
            // Default response for unsupported methods
            std::string response =
                "HTTP/1.1 405 Method Not Allowed\r\n"
                "Content-Length: 0\r\n"
                "\r\n";
            send(clientSocket, response.c_str(), response.size(), 0);
        }
    }
    else {
        std::cerr << "Failed to receive data from client" << std::endl;
    }
}

void Server::handleGetRequest(SOCKET clientSocket, const std::string& path) {
    // Parse query string if present
    size_t queryPos = path.find("?");
    std::string queryString;
    if (queryPos != std::string::npos) {
        queryString = path.substr(queryPos + 1);
    }

    // Parse query string into key-value pairs
    std::map<std::string, std::string> queryParams;
    std::istringstream queryStream(queryString);
    std::string pair;
    while (std::getline(queryStream, pair, '&')) {
        size_t equalsPos = pair.find("=");
        if (equalsPos != std::string::npos) {
            std::string key = pair.substr(0, equalsPos);
            std::string value = pair.substr(equalsPos + 1);
            queryParams[key] = value;
        }
    }

    // Print the parsed parameters
    for (const auto& param : queryParams) {
        std::cout << "Query Parameter: " << param.first << " = " << param.second << std::endl;
    }

    // Send response
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 13\r\n"
        "Hello, GET!";
    send(clientSocket, response.c_str(), response.size(), 0);
}


void Server::handlePostRequest(SOCKET clientSocket, std::istringstream& requestStream, const char* requestData) {
    std::string line;
    std::string contentLengthStr;
    size_t contentLength = 0;
     bool contentLengthFound = false;  // To track if Content-Length header is found

     // Parse headers to find Content-Length
     while (std::getline(requestStream, line) && line != "\r") {
         // Check for an empty line which marks the end of the headers
         std::cout << "Parsing Line: '" << line << "'" << std::endl;
         if (line.empty()) {
             break;  // End of headers
         }

         // Look for the Content-Length header
         if (line.find("Content-Length:") == 0) {
             contentLengthStr = line.substr(15);  // Get the value after "Content-Length:"
             contentLength = std::stoul(contentLengthStr);  // Convert to size_t
             contentLengthFound = true;
             contentLengthStr.erase(contentLengthStr.find_last_not_of(" \r\n") + 1);  // Trim spaces/newlines
         }
     }

     // Read the body
     std::string body;
     if (contentLength > 0) {
         char* bodyBuffer = new char[contentLength + 1];
         int totalBytesRead = 0;

         while (totalBytesRead < contentLength) {
             int bytesRead = recv(clientSocket, bodyBuffer + totalBytesRead, contentLength - totalBytesRead, 0);
             if (bytesRead <= 0) {
                 break; // Error or client disconnected
             }
             totalBytesRead += bytesRead;
         }

         bodyBuffer[totalBytesRead] = '\0';
         body = bodyBuffer; // Convert buffer to string
         delete[] bodyBuffer;
     }

     std::cout << "POST Body:\n" << body << std::endl;

    // Create a dynamic response including the received POST body
    std::string responseBody = "<html><body><h1>Received Data</h1><p>" + body + "</p></body></html>";
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(responseBody.size()) + "\r\n"
        "\r\n" +
        responseBody;

    // Send the response
    send(clientSocket, response.c_str(), response.size(), 0);
}
