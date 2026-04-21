#include "shared.h"
#include "quit_with_error.h"

// process one client request
void HandleClient(int clntSock)
{
    toSA clientRequest;
    toCA serverResponse;

    int recvMsgSize;

    // receive request from client
    if ((recvMsgSize = recv(clntSock, &clientRequest, sizeof(clientRequest), 0)) < 0)
        QuitWithError("recv() failed");

    printf("Server: received request from user %u\n", clientRequest.userID);

    // start building response
    memset(&serverResponse, 0, sizeof(serverResponse));
    serverResponse.userID = clientRequest.userID;
    serverResponse.category = clientRequest.category;
    serverResponse.offerID = clientRequest.offerID;

    // handle request type
    switch (clientRequest.messageType)
    {
    case login:
        printf("Server: processing login request\n");
        serverResponse.messageType = ackLogin;
        break;

    case createNotifier:
    {
        int i;
        int slotFound = -1;

        printf("Server: registering notifier for user %u\n", clientRequest.userID);

        // clean array before adding new notifier
        CleanUpNotifierArray();

        // remove old notifier for this user if one exists
        RemoveNotifierByUserID(clientRequest.userID);

        // find empty slot
        for (i = 0; i < MAX_NOTIFIERS; i++)
        {
            if (activeNotifiers[i].isLoggedIn == 0)
            {
                slotFound = i;
                break;
            }
        }

        // save notifier socket and send old matching offers
        if (slotFound != -1)
        {
            activeNotifiers[slotFound].socket = clntSock;
            activeNotifiers[slotFound].userID = clientRequest.userID;
            activeNotifiers[slotFound].isLoggedIn = 1;

            SendInitialNotifications(clntSock, clientRequest.userID);
            return;
        }
        else
        {
            printf("Server: max notifiers reached\n");
            close(clntSock);
            return;
        }
    }

    case offer:
        printf("Server: processing offer request\n");

        // assign new offer id
        clientRequest.offerID = GetNextOfferID();

        printf("Server: assigned offer id %u\n", clientRequest.offerID);
        printf("Server: item description %s\n", clientRequest.message);
        printf("Server: location %s\n", clientRequest.location);

        // save offer and notify matching users
        SaveOfferToFileDB(clientRequest);
        SendNotificationsToUsers(clientRequest);

        serverResponse.messageType = ackOffer;
        serverResponse.offerID = clientRequest.offerID;
        break;

    case request:
        printf("Server: processing item request\n");

        // save request category for this user
        SaveRequestToFileDB(clientRequest.userID, clientRequest.category);

        serverResponse.messageType = ackRequest;
        break;

    case removeRequest:
    {
        int removeSuccess;

        printf("Server: processing remove request\n");

        // remove request from file
        removeSuccess = RemoveRequestFromFileDB(clientRequest.userID, clientRequest.category);

        if (removeSuccess)
            printf("Server: remove request successful\n");
        else
            printf("Server: remove request failed\n");

        serverResponse.messageType = ackRemoveRequest;
        break;
    }

    case getList:
        printf("Server: processing get list request\n");

        // send matching offers and close socket in this function path
        GetOfferList(clntSock, clientRequest.category);
        close(clntSock);
        return;

    case removeOffer:
    {
        int removeSuccess;

        printf("Server: processing remove offer request\n");
        printf("Server: offer id %u\n", clientRequest.offerID);

        // remove offer from file
        removeSuccess = RemoveOfferFromFileDB(clientRequest.offerID, clientRequest.userID);

        if (removeSuccess)
            printf("Server: remove offer successful\n");
        else
            printf("Server: remove offer failed\n");

        serverResponse.messageType = ackRemoveOffer;
        break;
    }

    case logout:
        printf("Server: processing logout request\n");

        // remove notifier socket for this user
        RemoveNotifierByUserID(clientRequest.userID);

        serverResponse.messageType = ackLogout;
        break;

    default:
        printf("Server: unknown request type\n");

        // clean up notifier if needed
        for (int i = 0; i < MAX_NOTIFIERS; i++)
        {
            if (activeNotifiers[i].isLoggedIn && activeNotifiers[i].userID == clientRequest.userID)
            {
                activeNotifiers[i].isLoggedIn = 0;
                close(activeNotifiers[i].socket);
                break;
            }
        }

        CleanUpNotifierArray();

        serverResponse.messageType = ackLogout;
        break;
    }

    // send response
    if (send(clntSock, &serverResponse, sizeof(serverResponse), 0) < 0)
        QuitWithError("send() failed");

    printf("Server: sent response to client\n");

    // close request socket
    close(clntSock);
}