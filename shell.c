#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

const char* const prog_usage = "Usage: falsh [-h]\n";

// find_in_path finds searches the PATH for the given command and
// stores the full path in buffer if it isn't NULL.
// Returns true if the command is in the PATH, false otherwise.
bool find_in_path(char** buffer, const char* const command) {
    return false;
}

int main(int argc, char** argv) {
    // Error if more than two args or if the second arg isn't help
    if (argc > 2 || (argc == 2 && strcmp(argv[1], "-h") != 0)) {
        fprintf(stderr, "Invalid arguments\n%s", prog_usage);
        exit(EXIT_FAILURE);
    } else if (argc == 2) { // Second arg must be -h from above
        printf("%s", prog_usage);
        exit(EXIT_SUCCESS);
    }
    
    exit(EXIT_SUCCESS);
}
