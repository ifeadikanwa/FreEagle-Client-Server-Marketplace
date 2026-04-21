#include "shared.h"
#include "quit_with_error.h"

// save one offer to the offers file
void SaveOfferToFileDB(toSA clientRequest)
{
    FILE *offerFile;

    // open offers file in append mode
    offerFile = fopen("offers_db.txt", "a");
    if (offerFile == NULL)
        QuitWithError("fopen() failed");

    // write offer to file
    fprintf(offerFile, "%u|%u|%d|%s|%s\n",
            clientRequest.offerID,
            clientRequest.userID,
            clientRequest.category,
            clientRequest.message,
            clientRequest.location);

    fclose(offerFile);

    printf("Server: saved offer to file\n");
}