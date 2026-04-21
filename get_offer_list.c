#include "shared.h"
#include "quit_with_error.h"

// send all offers in one category
void GetOfferList(int clntSock, Category requestedCategory)
{
    FILE *offerFile;
    char line[256];

    int matchCount = 0;

    toCA serverResponse;

    unsigned int offerID;
    unsigned int userID;
    int category;
    char message[100];
    char location[20];

    // open offers file
    offerFile = fopen("offers_db.txt", "r");

    // send zero count if file does not exist
    if (offerFile == NULL)
    {
        printf("Server: offers file not found\n");

        memset(&serverResponse, 0, sizeof(serverResponse));
        serverResponse.messageType = ackListCount;
        serverResponse.offerID = 0;

        send(clntSock, &serverResponse, sizeof(serverResponse), 0);
        return;
    }

    // first pass to count matches
    while (fgets(line, sizeof(line), offerFile) != NULL)
    {
        if (sscanf(line, "%u|%u|%d|%99[^|]|%19[^\n]",
                   &offerID, &userID, &category, message, location) == 5)
        {
            if (category == requestedCategory)
                matchCount++;
        }
    }

    // send count first
    memset(&serverResponse, 0, sizeof(serverResponse));
    serverResponse.messageType = ackListCount;
    serverResponse.offerID = matchCount;

    if (send(clntSock, &serverResponse, sizeof(serverResponse), 0) < 0)
        QuitWithError("send() failed");

    printf("Server: sent list count %d\n", matchCount);

    // go back to start of file
    rewind(offerFile);

    // second pass to send matching offers
    while (fgets(line, sizeof(line), offerFile) != NULL)
    {
        if (sscanf(line, "%u|%u|%d|%99[^|]|%19[^\n]",
                   &offerID, &userID, &category, message, location) == 5)
        {
            if (category == requestedCategory)
            {
                // build one offer response
                memset(&serverResponse, 0, sizeof(serverResponse));

                serverResponse.messageType = ackList;
                serverResponse.offerID = offerID;
                serverResponse.userID = userID;
                serverResponse.category = requestedCategory;
                strcpy(serverResponse.message, message);
                strcpy(serverResponse.location, location);

                // send offer
                if (send(clntSock, &serverResponse, sizeof(serverResponse), 0) < 0)
                    QuitWithError("send() failed");

                printf("Server: sent offer id %u\n", offerID);
            }
        }
    }

    printf("Server: finished sending offer list.\n");

    fclose(offerFile);
}