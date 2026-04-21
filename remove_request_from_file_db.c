#include "shared.h"
#include "quit_with_error.h"

// remove one request from the requests file
int RemoveRequestFromFileDB(unsigned int userIDToRemove, Category categoryToRemove)
{
    FILE *requestFile;
    FILE *tempFile;

    char line[256];

    unsigned int userID;
    int category;

    int removeSuccess = 0;

    // open request file
    requestFile = fopen("requests_db.txt", "r");
    if (requestFile == NULL)
    {
        printf("Server: requests file not found\n");
        return 0;
    }

    // open temp file
    tempFile = fopen("temp_requests_db.txt", "w");
    if (tempFile == NULL)
        QuitWithError("fopen() failed");

    // copy all requests except the one being removed
    while (fgets(line, sizeof(line), requestFile) != NULL)
    {
        if (sscanf(line, "%u|%d", &userID, &category) == 2)
        {
            if (userID == userIDToRemove && category == categoryToRemove)
            {
                removeSuccess = 1;
                printf("Server: removed request for user %u\n", userIDToRemove);
            }
            else
            {
                fprintf(tempFile, "%u|%d\n", userID, category);
            }
        }
    }

    fclose(requestFile);
    fclose(tempFile);

    // replace old request file
    if (remove("requests_db.txt") != 0)
        QuitWithError("remove() failed");

    if (rename("temp_requests_db.txt", "requests_db.txt") != 0)
        QuitWithError("rename() failed");

    return removeSuccess;
}