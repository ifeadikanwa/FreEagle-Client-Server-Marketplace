#include "shared.h"
#include "quit_with_error.h"

// save one request if it does not already exist
void SaveRequestToFileDB(unsigned int userID, Category category)
{
    FILE *requestFile;
    char line[256];

    unsigned int savedUserID;
    int savedCategory;

    // open file first to check for duplicates
    requestFile = fopen("requests_db.txt", "r");

    if (requestFile != NULL)
    {
        while (fgets(line, sizeof(line), requestFile) != NULL)
        {
            if (sscanf(line, "%u|%d", &savedUserID, &savedCategory) == 2)
            {
                if (savedUserID == userID && savedCategory == category)
                {
                    fclose(requestFile);
                    printf("Server: request already exists\n");
                    return;
                }
            }
        }

        fclose(requestFile);
    }

    // open file in append mode
    requestFile = fopen("requests_db.txt", "a");
    if (requestFile == NULL)
        QuitWithError("fopen() failed");

    // save request
    fprintf(requestFile, "%u|%d\n", userID, category);

    fclose(requestFile);

    printf("Server: saved request to file\n");
}