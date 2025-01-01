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
        return -1;
    }

    // always set to true so we are always listening to clients.
    while (true) {
        server.acceptClient();
    }

    return 0;

}
