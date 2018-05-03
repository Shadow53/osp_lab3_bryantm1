#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "color.h"

const char* const prog_name = "falsh";
const char* const prog_usage = "Usage: falsh [-h]\n";
const char* const prog_help =   "SPU Falsh, version 0.0.1-alpha\n"
                                "These shell commands are defined internally. Type `help` to see this list.\n\n"
                                "help\n"
                                "exit\n"
                                "pwd\n"
                                "cd [dir]\n"
                                "setpath <dir> [dir ...]\n";
char* cwd = NULL;
const char* const delim = " \n\t\v\f\r";

int is_same_fd(int fd1, int fd2) {
    // Create two stat{} structs
    struct stat stat1, stat2;
    // Read file information into the structs and
    // return -1 if an error occurs
    if (fstat(fd1, &stat1) == -1) return -1;
    if (fstat(fd2, &stat2) == -1) return -1;
    // st_dev and st_ino together for a unique identifier for a file
    if ((stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino)) {
        return 1; // true but not -1
    } else {
        return 0; // false
    }
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

    // Store copies of stdout and stderr in case of redirection
    // The dup() function creates a copy of the given file descriptor
    const int stdout_save = dup(STDOUT_FILENO);
    const int stderr_save = dup(STDERR_FILENO);

    // Store if an error occurred
    bool has_error = false;

    // Initially set PATH to /bin
    if (setenv("PATH", "/bin", true) == -1) {
        // Print error if error occurred
        perror("falsh: PATH");
        // Error occurred
        has_error = true;
    }

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

            // Check for redirection before tokenizing
            // If redirecting, set up redirection
            // redir_file is a pointer to the filename to redirect to
            char* redir_file = NULL;
            if ((redir_file = strstr(line, " > ")) != NULL) {
                // Terminate line before the redirection arguments
                // by adding a null terminator
                redir_file[0] = 0;
                // Ignore the leading " > " in the string
                redir_file += 3;
                // Get the file argument from redir_file as a token
                char* redir_filename = strtok(redir_file, delim);
                // Make sure file was the only argument
                // If not, print an error and prevent further processing
                // by null-terminating line so the later strtok() call returns null
                if (strtok(NULL, delim) != NULL) {
                    fprintf(stderr, "falsh: provide one file name for redirection\n");
                    line[0] = 0;
                    has_error = true;
                } else {
                    // Otherwise, set up redirection
                    // Build strings for the file names
                    int name_len = strlen(redir_filename);
                    char* out_file_name = (char*)malloc((name_len + 5) * sizeof(char));
                    char* err_file_name = (char*)malloc((name_len + 5) * sizeof(char));
                    // Copy the name prefix to both strings using strncpy
                    strcpy(out_file_name, redir_filename);
                    strcpy(err_file_name, redir_filename);
                    // Append prefixes to the file names
                    strcat(out_file_name, ".out");
                    strcat(err_file_name, ".err");
                    // Open file descriptors for each file
                    // creat() takes the path to the file and mode arguments
                    // In this case, the files are created with 0644 permissions
                    int out_file = creat(out_file_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    // Check if file opening failed, error if so
                    if (out_file == -1) {
                        // Print the error and disable continuing by truncating line
                        perror("falsh: redirect (out)");
                        has_error = true;
                    } else {
                        int err_file = creat(err_file_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                        if (err_file == -1) {
                            // Print the error and mark that an error occurred
                            perror("falsh: redirect (err)");
                            has_error = true;
                            // Close the out file, since we won't use it
                            close(out_file);
                        } else {
                            // No errors, redirect outputs
                            // dup2() copies the file descriptor of the first
                            // argument to the one at the second argument
                            if (dup2(out_file, STDOUT_FILENO) == -1) {
                                // Failed to redirect stdout file descriptor
                                perror("falsh: failed to redirect stdout");
                            } else if (dup2(err_file, STDERR_FILENO) == -1) {
                                // Failed to redirect stderr file descriptor
                                perror("falsh: failed to redirect stderr");
                            }
                            // Don't need the file descriptors after copying,
                            // close them now.
                            close(out_file);
                            close(err_file);
                        }
                    }

                    if (has_error) {
                        // Force command parsing to not happen by truncating line
                        line[0] = 0;
                    }
                    
                    // Free allocated memory
                    free(out_file_name);
                    free(err_file_name);
                }
            }

            // Tokenize the input string
            char* token = NULL;
            // strtok returns the first token in the string,
            // delimited by a space character, or NULL if the string
            // is empty or is all delimiters.
            token = strtok(line, delim);

            // Check values of token
            if (token != NULL) {
                if (strcmp(token, "help") == 0) {
                    // Print the help text.
                    // No check for any extra arguments, because if the user
                    // is asking for help, we should give it regardless of
                    // how confused they are.
                    printf("%s", prog_help);
                } else if (strcmp(token, "exit") == 0) {
                    // Make sure the full command was just "exit"
                    // Passing NULL tells it to use the previous string
                    token = strtok(NULL, delim);
                    if (token == NULL) {
                        exit(EXIT_SUCCESS);
                    } else {
                        // bash prints an error message and exits anyways,
                        // just with a failing exit code.
                        fprintf(stderr, "%s: exit takes no arguments\n", prog_name);
                        exit(EXIT_FAILURE);
                    }
                } else if (strcmp(token, "pwd") == 0) {
                    // Make sure the full command was just "pwd"
                    // Passing NULL tells it to use the previous string
                    token = strtok(NULL, delim);
                    if (token == NULL) {
                        // We already have the current working directory: print it
                        printf("%s\n", cwd);
                    } else {
                        // This is the error that bash prints. It's the right error,
                        // so we'll do it the same way.
                        fprintf(stderr, "pwd: too many arguments\n");
                        has_error = true;
                    }
                } else if (strcmp(token, "cd") == 0) {
                    // Check if a directory was given
                    token = strtok(NULL, delim);
                    // Use "HOME" if no directory given, otherwise use the given one
                    char* dir = (token == NULL ? getenv("HOME") : token);
                    // Check if any other arguments were given
                    token = strtok(NULL, delim);
                    if (token != NULL) {
                        fprintf(stderr, "cd: too many arguments\n");
                        has_error = true;
                    } else if (chdir(dir) == -1) {
                        // Call to `cd` was valid but an error occurred
                        // Print the error message associated with errno
                        // The argument to perror() is the prefix to the error message
                        perror("cd");
                        has_error = true;
                    }
                } else if (strcmp(token, "setpath") == 0) {
                    if ((token = strtok(NULL, delim)) == NULL) {
                        fprintf(stderr, "setpath: need at least one argument");
                    } else {
                        if (setenv("PATH", token, true) == -1) {
                            // Error while setting PATH, print error
                            perror("setpath");
                            has_error = true;
                        }
                        // Loop through all arguments given to setenv and add to path
                        while((token = strtok(NULL, delim)) != NULL) {
                            // Get the current value of PATH
                            char* path = getenv("PATH");
                            // Allocate a string to append to
                            // If PATH was set in a previous iteration, we want
                            // to add a ':' between the paths, so add another
                            // character to fit that.
                            char* new_path = (char*)malloc((strlen(token) +
                                (path == NULL ? 1 : strlen(path) + 2)) * sizeof(char));
                            // Add null terminator to first element to ensure
                            // strcat works correctly even if this is the first
                            // directory in PATH.
                            new_path[0] = 0;
                            // Init new_path to the previous iteration of PATH,
                            // if applicable
                            if (path != NULL) {
                                // Copy the previous path to new_path
                                strcpy(new_path, path);
                                // Add a colon to separate paths
                                strcat(new_path, ":");
                            }
                            // Concatonate the current PATH directory to PATH
                            strcat(new_path, token);
                            // Set new PATH - check for errors
                            if (setenv("PATH", new_path, true) == -1) {
                                // Print error
                                perror("setpath");
                                has_error = true;
                            }
                        }
                    }
                } else {
                    // It was none of the built-in commands, try running from $PATH
                    // Fork child
                    int fork_rc = fork();
                    if (fork_rc == -1) {
                        // Fork failed, print error and be done
                        perror("falsh");
                        has_error = true;
                    } else if (fork_rc == 0) {
                        // Allocate an array of arguments
                        int len = 1;
                        // Allocate array of strings for args
                        char** eargs = (char**)malloc(sizeof(char*));
                        // Set first arg equal to command name
                        eargs[0] = token;
                        while ((token = strtok(NULL, delim)) != NULL) {
                            // Increment length
                            ++len;
                            // Reallocate memory
                            if (realloc(eargs, len * sizeof(char*)) == NULL) {
                                // Reallocation failed
                                perror(eargs[0]);
                                has_error = true;
                                break;
                            }
                            // Assign next arg
                            eargs[len-1] = token;
                        }
                        // NULL-terminate the list
                        if (realloc(eargs, (len+1) * sizeof(char*)) == NULL) {
                            // Reallocation failed
                            perror(eargs[0]);
                            has_error = true;
                        } else {
                            eargs[len] = NULL;
                        }

                        if (!has_error) {
                            // exec() the program
                            execvp(eargs[0], eargs);
                        }
                        // This only does anything if an error occurs
                        perror(eargs[0]);
                        has_error = true;
                        exit(EXIT_FAILURE);
                    } else {
                        // child_status will hold the exit code of the forked child
                        int child_status;
                        // Wait for only child to exit
                        int wc = wait(&child_status);
                        // Check the status code
                        // WIFEXITED is true if child exited normally
                        if (WIFEXITED(child_status)) {
                            // WEXITSTATUS returns the code returned by the child
                            // 0 is success, so anything else was an error
                            has_error = WEXITSTATUS(child_status) != 0;
                        }
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

        // Restore stdout and stderr
        // Don't need to check if they were changed, because
        // dup2()-ing something to itself does nothing
        // What happens if I print an error to stderr with the
        // redirection, because I can't undo the redirection?

        // First, flush the buffers
        int rc;
        // Check if these are the same 
        if ((rc = is_same_fd(stdout_save, STDOUT_FILENO)) == -1) {
            // An error occurred while stat-ing a file
            perror("falsh");
            has_error = true;
        } else if (rc == 0 && fsync(STDOUT_FILENO) == -1) {
            // Only flush if NOT the same
            // Failed to flush stdout to disc
            perror("falsh: failed to flush output to disc");
            has_error = true;
        }

        // Check if these are the same
        if ((rc = is_same_fd(stderr_save, STDERR_FILENO)) == -1) {
            // An error occurred while stat-ing a file
            perror("falsh");
            has_error = true;
        } else if (rc == 0 && fsync(STDERR_FILENO) == -1) {
            // Only flush if NOT the same
            // Failed to flush stderr to disc
            perror("falsh: failed to flush errors to disc");
            has_error = true;
        }

        if (dup2(stdout_save, STDOUT_FILENO) == -1) {
            // Failed to restore stdout
            perror("falsh: failed to restore stdout");
            has_error = true;
        }
        if (dup2(stderr_save, STDERR_FILENO) == -1) {
            // Failed to restore stderr
            perror("falsh: failed to restore stderr");
            has_error = true;
        }

        // Print the shell prompt:
        // username /current/working/dir $
        printf("%s%s ", has_error ? ANSI_FG_BRIGHT_RED : ANSI_FG_BRIGHT_BLUE, getlogin());
        printf("%s%s %s$%s ", ANSI_FG_BRIGHT_BLACK, cwd, ANSI_FG_GREEN, ANSI_RESET);
        // Free malloc'd memory and set line to NULL again
        free(line);
        line = NULL;
        // Reset has_error
        has_error = false;
    } while ((nread = getline(&line, &n, stdin)) != -1);

    exit(EXIT_SUCCESS);
}
