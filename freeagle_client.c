#include "shared.h"
#include "quit_with_error.h"
#include <signal.h>

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in servAddr;
    unsigned short servPort;
    char *servIP;

    toSA clientRequest;
    toCA serverResponse;

    int currentUserID;
    char password[50];

    int loggedIn = 0;
    int menuChoice;
    int c;

    pid_t pid = -1;

    // check command line arguments
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];
    servPort = atoi(argv[2]);

    // login prompt
    printf("Welcome to FreEagle\n");
    printf("Enter userID:");
    scanf("%d", &currentUserID);

    printf("Enter password:");
    scanf("%49s", password);

    // create login socket
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        QuitWithError("socket() failed");

    // build server address
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(servIP);
    servAddr.sin_port = htons(servPort);

    // connect to server
    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        QuitWithError("connect() failed");

    printf("Client: connected to server %s on port %d\n", servIP, servPort);

    // build login request
    memset(&clientRequest, 0, sizeof(clientRequest));
    clientRequest.messageType = login;
    clientRequest.userID = currentUserID;

    // send login request
    if (send(sock, &clientRequest, sizeof(clientRequest), 0) < 0)
        QuitWithError("send() failed");

    printf("Client: sent login request for user %u\n", clientRequest.userID);

    // receive login response
    if (recv(sock, &serverResponse, sizeof(serverResponse), 0) < 0)
        QuitWithError("recv() failed");

    if (serverResponse.messageType == ackLogin)
    {
        loggedIn = 1;
        printf("Client: login successful\n");
    }
    else
    {
        printf("Client: login failed\n");
    }

    // close login socket
    close(sock);

    // fork after login so child can wait for notifications
    if (loggedIn)
    {
        pid = fork();

        if (pid < 0)
        {
            QuitWithError("fork() failed");
        }
        else if (pid == 0)
        {
            int notifierSock;
            toSA notifierRequest;
            toCA rcvdNotification;

            // create notifier socket
            if ((notifierSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
                QuitWithError("socket() failed in child");

            // connect notifier socket to server
            if (connect(notifierSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
                QuitWithError("connect() failed in child");

            // register notifier socket
            memset(&notifierRequest, 0, sizeof(notifierRequest));
            notifierRequest.messageType = createNotifier;
            notifierRequest.userID = currentUserID;

            if (send(notifierSock, &notifierRequest, sizeof(notifierRequest), 0) < 0)
                QuitWithError("send() failed for createNotifier");

            // wait forever for notifications
            for (;;)
            {
                if (recv(notifierSock, &rcvdNotification, sizeof(rcvdNotification), 0) > 0 &&
                    rcvdNotification.messageType == notification)
                {
                    // print notification
                    printf("\n\n ***** OFFER NOTIFICATION ***** \n");
                    printf(" Category: %s\n", convertCategoryToString(rcvdNotification.category));
                    printf(" Description: %s\n", rcvdNotification.message);
                    printf(" Location: %s\n", rcvdNotification.location);
                    printf("\nClient: check the menu prompt above and continue your choice\n");
                    fflush(stdout);
                }
                else
                {
                    // stop if server disconnects
                    break;
                }
            }

            close(notifierSock);
            exit(0);
        }
    }

    // main client menu loop
    while (loggedIn)
    {
        // i use this to clear any leftover input before i scan 
        // while ((c = getchar()) != '\n' && c != EOF);
            
        int categoryChoice;

        // show menu
        printf("\nFreEagle Menu:\n");
        printf("1. offer\n");
        printf("2. request\n");
        printf("3. get list\n");
        printf("4. remove request\n");
        printf("5. remove offer\n");
        printf("6. logout\n");
        printf("Enter choice: ");
        while ((c = getchar()) != '\n' && c != EOF)
            ;
        scanf("%d", &menuChoice);

        // clear request and store current user
        memset(&clientRequest, 0, sizeof(clientRequest));
        clientRequest.userID = currentUserID;

        // build request based on menu choice
        switch (menuChoice)
        {
        case 1:
            clientRequest.messageType = offer;

            printf("Choose category\n");
            printf("1. supplies\n");
            printf("2. books\n");
            printf("3. furniture\n");
            printf("4. misc\n");
            printf("Enter category: ");
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            scanf("%d", &categoryChoice);

            switch (categoryChoice)
            {
            case 1:
                clientRequest.category = supplies;
                break;
            case 2:
                clientRequest.category = books;
                break;
            case 3:
                clientRequest.category = furniture;
                break;
            case 4:
                clientRequest.category = misc;
                break;
            default:
                printf("Client: invalid category\n");
                continue;
            }

            printf("Enter item description: ");
            scanf(" %99[^\n]", clientRequest.message);

            printf("Enter location: ");
            scanf(" %19[^\n]", clientRequest.location);
            break;

        case 2:
            clientRequest.messageType = request;

            printf("Choose category\n");
            printf("1. supplies\n");
            printf("2. books\n");
            printf("3. furniture\n");
            printf("4. misc\n");
            printf("Enter category: ");
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            scanf("%d", &categoryChoice);

            switch (categoryChoice)
            {
            case 1:
                clientRequest.category = supplies;
                break;
            case 2:
                clientRequest.category = books;
                break;
            case 3:
                clientRequest.category = furniture;
                break;
            case 4:
                clientRequest.category = misc;
                break;
            default:
                printf("Client: invalid category\n");
                continue;
            }
            break;

        case 3:
            clientRequest.messageType = getList;

            printf("Choose category\n");
            printf("1. supplies\n");
            printf("2. books\n");
            printf("3. furniture\n");
            printf("4. misc\n");
            printf("Enter category: ");
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            scanf("%d", &categoryChoice);

            switch (categoryChoice)
            {
            case 1:
                clientRequest.category = supplies;
                break;
            case 2:
                clientRequest.category = books;
                break;
            case 3:
                clientRequest.category = furniture;
                break;
            case 4:
                clientRequest.category = misc;
                break;
            default:
                printf("Client: invalid category. try again\n");
                continue;
            }
            break;

        case 4:
            clientRequest.messageType = removeRequest;

            printf("Choose category\n");
            printf("1. supplies\n");
            printf("2. books\n");
            printf("3. furniture\n");
            printf("4. misc\n");
            printf("Enter category: ");
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            scanf("%d", &categoryChoice);

            switch (categoryChoice)
            {
            case 1:
                clientRequest.category = supplies;
                break;
            case 2:
                clientRequest.category = books;
                break;
            case 3:
                clientRequest.category = furniture;
                break;
            case 4:
                clientRequest.category = misc;
                break;
            default:
                printf("Client: invalid category\n");
                continue;
            }
            break;

        case 5:
            clientRequest.messageType = removeOffer;

            printf("Enter offer ID: ");
            scanf("%u", &clientRequest.offerID);
            break;

        case 6:
            clientRequest.messageType = logout;
            break;

        default:
            printf("Client: invalid choice\n");
            continue;
        }

        // create request socket
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            QuitWithError("socket() failed");

        // rebuild server address
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = inet_addr(servIP);
        servAddr.sin_port = htons(servPort);

        // connect to server
        if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
            QuitWithError("connect() failed");

        printf("\nClient: connected to server %s on port %d\n", servIP, servPort);

        // send request
        if (send(sock, &clientRequest, sizeof(clientRequest), 0) < 0)
            QuitWithError("send() failed");

        printf("Client: sent request for user %u\n", clientRequest.userID);

        // receive response
        if (recv(sock, &serverResponse, sizeof(serverResponse), 0) < 0)
            QuitWithError("recv() failed");

        printf("Client: received response from server for user %u\n", clientRequest.userID);

        // handle response type
        switch (serverResponse.messageType)
        {
        case ackLogin:
            printf("Client: received ack login\n");
            break;

        case ackOffer:
            printf("Client: received ack offer\n");
            printf("Client: offer id %u\n", serverResponse.offerID);
            break;

        case ackRequest:
            printf("Client: received ack request\n");
            break;

        case ackList:
            printf("Client: ERROR. received ack list without ack list count\n");
            break;

        case ackListCount:
        {
            int i;
            unsigned int count = serverResponse.offerID;

            printf("Client: total offers %u\n", count);

            // receive all matching offers
            for (i = 0; i < count; i++)
            {
                if (recv(sock, &serverResponse, sizeof(serverResponse), 0) < 0)
                    QuitWithError("recv() failed");

                if (serverResponse.messageType == ackList)
                {
                    printf("\nClient: Offer Details\n");
                    printf("id: %u\n", serverResponse.offerID);
                    printf("user: %u\n", serverResponse.userID);
                    printf("category: %s\n", convertCategoryToString(serverResponse.category));
                    printf("description: %s\n", serverResponse.message);
                    printf("location: %s\n", serverResponse.location);
                }
            }

            printf("\nClient: end of offers list.\n");
            break;
        }

        case ackRemoveRequest:
            printf("Client: received ack remove request\n");
            break;

        case ackRemoveOffer:
            printf("Client: received ack remove offer\n");
            break;

        case notification:
            printf("Client: received notification\n");
            break;

        case ackLogout:
            printf("Client: received ack logout\n");
            loggedIn = 0;
            break;

        default:
            printf("Client: received unknown response\n");
            break;
        }

        // close request socket
        close(sock);
    }

    // stop child before exiting
    if (pid > 0)
    {
        printf("Client: Shutting down notification listener...\n");
        kill(pid, SIGKILL);
    }

    return 0;
}