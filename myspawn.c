#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

extern char **environ;

// List of allowed commands
const char *allowed_commands[] = {
    "cp", "touch", "mkdir", "ls", "pwd", "cat", "grep", 
    "chmod", "diff", "cd", "exit", "help", NULL
};

void execute_external_command(char *cmd, char **args);
void handle_cd(char **args);
void print_help();

int main() {
    pid_t pid;
    char *argv[21]; // Max of 20 arguments plus NULL terminator
    int status;
    posix_spawnattr_t attr;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (1) {
        // Prompt the user for input
        printf("rsh> ");
        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            // If the user presses Ctrl+D or EOF, exit gracefully
            printf("\nExiting rsh...\n");
            break;
        }

        // Remove the newline character
        line[strlen(line) - 1] = '\0';

        // Tokenize the input into individual arguments
        int i = 0;
        char *token = strtok(line, " ");
        while (token != NULL && i < 20) {
            argv[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        argv[i] = NULL;

        // Check for built-in commands
        if (argv[0] == NULL) {
            continue; // Empty input, just show prompt again
        } else if (strcmp(argv[0], "exit") == 0) {
            // Exit the shell
            break;
        } else if (strcmp(argv[0], "help") == 0) {
            // Print help
            print_help();
        } else if (strcmp(argv[0], "cd") == 0) {
            // Handle the 'cd' command
            handle_cd(argv);
        } else {
            // Check if the command is allowed
            int is_allowed = 0;
            for (int j = 0; allowed_commands[j] != NULL; j++) {
                if (strcmp(argv[0], allowed_commands[j]) == 0) {
                    is_allowed = 1;
                    break;
                }
            }
            
            if (is_allowed) {
                // Execute the external command
                execute_external_command(argv[0], argv);
            } else {
                printf("NOT ALLOWED!\n");
            }
        }
    }

    // Cleanup
    free(line);
    return 0;
}

// Function to execute an external command using posix_spawnp
void execute_external_command(char *cmd, char **args) {
    pid_t pid;
    int status;
    posix_spawnattr_t attr;

    // Initialize spawn attributes
    posix_spawnattr_init(&attr);

    // Spawn the command
    if (posix_spawnp(&pid, cmd, NULL, &attr, args, environ) != 0) {
        perror("spawn failed");
        return;
    }

    // Wait for the command to finish
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
    }

    if (WIFEXITED(status)) {
        printf("Spawned process exited with status %d\n", WEXITSTATUS(status));
    }

    // Destroy spawn attributes
    posix_spawnattr_destroy(&attr);
}

// Function to handle the 'cd' command
void handle_cd(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        // If there are too many or too few arguments, print an error
        printf("-rsh: cd: too many arguments\n");
    } else if (chdir(args[1]) != 0) {
        // Try to change directory
        perror("cd failed");
    }
}

// Function to print the help message
void print_help() {
    printf("The allowed commands are:\n");
    for (int i = 0; allowed_commands[i] != NULL; i++) {
        printf("%d: %s\n", i + 1, allowed_commands[i]);
    }
}
