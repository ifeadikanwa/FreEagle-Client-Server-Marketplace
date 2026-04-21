#include "shared.h"
#include "quit_with_error.h"

// get next offer id and update file
unsigned int GetNextOfferID()
{
    FILE *file;
    unsigned int id = 1;

    // open id file for reading
    file = fopen("next_offer_id.txt", "r");

    if (file != NULL)
    {
        fscanf(file, "%u", &id);
        fclose(file);
    }

    // open id file for writing
    file = fopen("next_offer_id.txt", "w");
    if (file == NULL)
        QuitWithError("fopen() failed");

    // save next id for future offer
    fprintf(file, "%u", id + 1);
    fclose(file);

    return id;
}