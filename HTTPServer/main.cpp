#include "Server.h"

int main()
{
    Server server(8080);

    if (!server.initialize()) {
        return 1;
    }

    if (!server.bindSocket()) {
        return 1;
    }

    if (!server.startListening()) {
        return 1;
    }

    while (true) {
        server.acceptClient();
    }

    return 0;

}
