#include "shared.h"
#include "quit_with_error.h"

// remove one offer if it belongs to the current user
int RemoveOfferFromFileDB(unsigned int offerIDToRemove, unsigned int userIDToRemove)
{
    FILE *offerFile;
    FILE *tempFile;

    char line[256];

    unsigned int offerID;
    unsigned int userID;
    int category;
    char message[100];
    char location[20];

    int removeSuccess = 0;

    // open offers file
    offerFile = fopen("offers_db.txt", "r");
    if (offerFile == NULL)
    {
        printf("Server: offers file not found\n");
        return 0;
    }

    // open temp file
    tempFile = fopen("temp_offers_db.txt", "w");
    if (tempFile == NULL)
        QuitWithError("fopen() failed");

    // copy all offers except the one being removed
    while (fgets(line, sizeof(line), offerFile) != NULL)
    {
        if (sscanf(line, "%u|%u|%d|%99[^|]|%19[^\n]",
                   &offerID, &userID, &category, message, location) == 5)
        {
            if (offerID == offerIDToRemove)
            {
                // only remove if the current user owns the offer
                if (userID == userIDToRemove)
                {
                    removeSuccess = 1;
                    printf("Server: removed offer id %u\n", offerIDToRemove);
                }
                else
                {
                    printf("Server: remove unsuccessful because offer belongs to user %u\n", userID);

                    // keep offer if wrong user tried to remove it
                    fprintf(tempFile, "%u|%u|%d|%s|%s\n",
                            offerID,
                            userID,
                            category,
                            message,
                            location);
                }
            }
            else
            {
                fprintf(tempFile, "%u|%u|%d|%s|%s\n",
                        offerID,
                        userID,
                        category,
                        message,
                        location);
            }
        }
    }

    fclose(offerFile);
    fclose(tempFile);

    // replace old offers file
    if (remove("offers_db.txt") != 0)
        QuitWithError("remove() failed");

    if (rename("temp_offers_db.txt", "offers_db.txt") != 0)
        QuitWithError("rename() failed");

    return removeSuccess;
}