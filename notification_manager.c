#include "shared.h"

// send old matching offers when notifier first connects
void SendInitialNotifications(int clntSock, unsigned int userID)
{
    FILE *requestFile;
    FILE *offerFile;
    char line[256];

    // track categories this user requested
    int requestedCategories[4] = {0, 0, 0, 0};

    unsigned int savedUserID;
    int savedCategory;

    // read request file and mark requested categories
    requestFile = fopen("requests_db.txt", "r");
    if (requestFile != NULL)
    {
        while (fgets(line, sizeof(line), requestFile) != NULL)
        {
            if (sscanf(line, "%u|%d", &savedUserID, &savedCategory) == 2)
            {
                if (savedUserID == userID && savedCategory >= 0 && savedCategory <= 3)
                {
                    requestedCategories[savedCategory] = 1;
                }
            }
        }
        fclose(requestFile);
    }

    // send old offers that match requested categories
    offerFile = fopen("offers_db.txt", "r");
    if (offerFile != NULL)
    {
        unsigned int offerID, offerUserID;
        int category;
        char message[100];
        char location[20];

        while (fgets(line, sizeof(line), offerFile) != NULL)
        {
            if (sscanf(line, "%u|%u|%d|%99[^|]|%19[^\n]",
                       &offerID, &offerUserID, &category, message, location) == 5)
            {
                if (category >= 0 && category <= 3 && requestedCategories[category] == 1)
                {
                    // do not notify user about their own offer
                    if (offerUserID != userID)
                    {
                        toCA notificationMsg;
                        memset(&notificationMsg, 0, sizeof(notificationMsg));

                        // build notification message
                        notificationMsg.messageType = notification;
                        notificationMsg.offerID = offerID;
                        notificationMsg.userID = offerUserID;
                        notificationMsg.category = category;
                        strcpy(notificationMsg.message, message);
                        strcpy(notificationMsg.location, location);

                        // send notification
                        send(clntSock, &notificationMsg, sizeof(notificationMsg), 0);

                        printf("Server: Sent existing offer %u in requested category to User %u\n", offerID, userID);
                    }
                }
            }
        }
        fclose(offerFile);
    }
}

// check if a user requested a category
int CheckIfUserRequestedCategory(unsigned int userID, Category category)
{
    FILE *requestFile = fopen("requests_db.txt", "r");
    if (requestFile == NULL)
        return 0;

    char line[256];
    unsigned int savedUserID;
    int savedCategory;

    // search request file for matching user and category
    while (fgets(line, sizeof(line), requestFile) != NULL)
    {
        if (sscanf(line, "%u|%d", &savedUserID, &savedCategory) == 2)
        {
            if (savedUserID == userID && savedCategory == category)
            {
                fclose(requestFile);
                return 1;
            }
        }
    }

    fclose(requestFile);
    return 0;
}

// send live notifications for a new offer
void SendNotificationsToUsers(toSA clientRequest)
{
    toCA notificationMsg;

    // clean notifier array before sending
    CleanUpNotifierArray();

    // build notification message
    memset(&notificationMsg, 0, sizeof(notificationMsg));
    notificationMsg.messageType = notification;
    notificationMsg.offerID = clientRequest.offerID;
    notificationMsg.userID = clientRequest.userID;
    notificationMsg.category = clientRequest.category;
    strcpy(notificationMsg.message, clientRequest.message);
    strcpy(notificationMsg.location, clientRequest.location);

    // loop through notifier array
    for (int i = 0; i < MAX_NOTIFIERS; i++)
    {
        // only send if user requested this category
        if (CheckIfUserRequestedCategory(activeNotifiers[i].userID, clientRequest.category))
        {
            // do not notify the user who posted the offer
            if (activeNotifiers[i].userID != clientRequest.userID && activeNotifiers[i].isLoggedIn)
            {
                printf("Server: Sending alert to User %u on socket %d\n",
                       activeNotifiers[i].userID, activeNotifiers[i].socket);

                // send notification
                if (send(activeNotifiers[i].socket, &notificationMsg, sizeof(notificationMsg), 0) < 0)
                {
                    printf("Server: ERROR - failed to notify socket %d\n", activeNotifiers[i].socket);

                    // mark bad notifier as logged out
                    activeNotifiers[i].isLoggedIn = 0;
                    close(activeNotifiers[i].socket);

                    printf("Server: cleaned up dead socket %d\n", activeNotifiers[i].socket);
                }
            }
        }
    }

    // clean array again after failed sends
    CleanUpNotifierArray();
}

// shift active notifier users to the front
void CleanUpNotifierArray()
{
    int index;
    int emptySlotPointer = 0;

    // move logged in users to the front
    for (index = 0; index < MAX_NOTIFIERS; index++)
    {
        if (activeNotifiers[index].isLoggedIn)
        {
            if (emptySlotPointer != index)
            {
                activeNotifiers[emptySlotPointer] = activeNotifiers[index];
            }

            emptySlotPointer++;
        }
    }

    // clear remaining array slots
    while (emptySlotPointer < MAX_NOTIFIERS)
    {
        activeNotifiers[emptySlotPointer].socket = -1;
        activeNotifiers[emptySlotPointer].userID = 0;
        activeNotifiers[emptySlotPointer].isLoggedIn = 0;
        emptySlotPointer++;
    }
}

// remove notifier socket for a user
void RemoveNotifierByUserID(unsigned int userID)
{
    int i;

    // mark matching notifier as empty
    for (i = 0; i < MAX_NOTIFIERS; i++)
    {
        if (activeNotifiers[i].isLoggedIn && activeNotifiers[i].userID == userID)
        {
            close(activeNotifiers[i].socket);
            activeNotifiers[i].socket = -1;
            activeNotifiers[i].userID = 0;
            activeNotifiers[i].isLoggedIn = 0;
        }
    }

    // clean array after removal
    CleanUpNotifierArray();
}