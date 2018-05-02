#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "unistd.h"

const char* const prog_name = "falsh";
const char* const prog_usage = "Usage: falsh [-h]\n";
char* cwd = NULL;
const char* const delim = " \n\t\v\f\r";


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
            // In the upcoming call to strtok, we don't care about
            // the trailing newline, which will cause strtok to return
            // the newline instead of NULL. Replace the newline with NULL
            if (line[strlen(line) - 1] == '\n') {
                line[strlen(line) - 1] = 0;
            }

            // Tokenize the input string
            char* token = NULL;
            // strtok returns the first token in the string,
            // delimited by a space character, or NULL if the string
            // is empty or is all delimiters.
            token = strtok(line, delim);

            // Check values of token
            if (token != NULL) {
                if (strcmp(token, "exit") == 0) {
                    // Make sure the full command was just "exit"
                    token = strtok(NULL, delim);
                    if (token == NULL) {
                        exit(EXIT_SUCCESS);
                    } else {
                        // bash prints an error message and exits anyways,
                        // just with a failing exit code.
                        fprintf(stderr, "%s: exit takes no arguments\n", prog_name);
                        exit(EXIT_FAILURE);
                    }
                }
            }
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
