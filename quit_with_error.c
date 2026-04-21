#include <stdio.h>
#include <stdlib.h>
#include "quit_with_error.h"

// print error message and exit
void QuitWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}