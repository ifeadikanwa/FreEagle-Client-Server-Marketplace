#include "shared.h"

// convert category enum to text for printing
const char *convertCategoryToString(Category category)
{
    switch (category)
    {
        case supplies:
            return "supplies";
        case books:
            return "books";
        case furniture:
            return "furniture";
        case misc:
            return "misc";
        default:
            return "unknown";
    }
}