#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "unistd.h"

const char* const prog_usage = "Usage: falsh [-h]\n";
char* cwd = NULL;


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

    // Variables for reading from stdin (user commands)
    // line  - Contains the line of input after calling getline
    // n     - Contains the number of characters in line
    // nread - Contains the number of characters read by getline
    char* line = NULL;
    size_t n = 0;
    ssize_t nread = 0;

    // do-while prints the prompt once before reading input
    do {
        // Only parse input if there is any input
        // This check basically avoids issues on first iteration
        if (line != NULL) {
            // Do some stuff and be useful
        }

        // Get current working directory
        // Free malloc'd memory first
        if (cwd != NULL) {
            free(cwd);
            cwd = NULL;
        }
        cwd = getcwd(cwd, 0);

        // Print the shell prompt:
        // username /current/working/dir $
        printf("%s %s $ ", getlogin(), cwd);
    } while ((nread = getline(&line, &n, stdin)) != -1);

    exit(EXIT_SUCCESS);
}
