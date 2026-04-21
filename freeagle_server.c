#include "shared.h"
#include "quit_with_error.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXPENDING 5

// store active notifier sockets for logged in users
NotifierClient activeNotifiers[MAX_NOTIFIERS];

int main(int argc, char *argv[])
{
    int servSock;
    int clntSock;
    struct sockaddr_in servAddr;
    struct sockaddr_in clntAddr;
    unsigned short servPort;
    unsigned int clntLen;

    // set all notifier slots to empty
    for (int i = 0; i < MAX_NOTIFIERS; i++)
    {
        activeNotifiers[i].socket = -1;
        activeNotifiers[i].userID = 0;
        activeNotifiers[i].isLoggedIn = 0;
    }

    // check command line arguments
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }

    servPort = atoi(argv[1]);

    // create socket
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        QuitWithError("socket() failed");

    // build server address
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    // bind socket
    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        QuitWithError("bind() failed");

    // listen for clients
    if (listen(servSock, MAXPENDING) < 0)
        QuitWithError("listen() failed");

    printf("Server: running on port %d\n", servPort);

    // main server loop
    for (;;)
    {
        clntLen = sizeof(clntAddr);

        // accept client
        if ((clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntLen)) < 0)
            QuitWithError("accept() failed");

        printf("\nServer: connected to client %s\n", inet_ntoa(clntAddr.sin_addr));

        // handle one client request
        HandleClient(clntSock);
    }

    return 0;
}