#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12

extern char **environ;

// List of allowed commands
char *allowed[N] = {
    "cp", "touch", "mkdir", "ls", "pwd", "cat", "grep", "chmod", "diff", "cd", "exit", "help"
};

// Function to check if a command is allowed
int isAllowed(const char *cmd) {
    for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            return 1;  // Command is allowed
        }
    }
    return 0;  // Command is not allowed
}

void execute_external_command(char *cmd, char **args);
void handle_cd(char **args);
void print_help();

int main() {
    char line[256];  // Buffer to hold user input

    while (1) {
        // Display the shell prompt
        printf("rsh> ");
        fflush(stdout);  // Ensure prompt is displayed immediately

        // Read input from the user
        if (fgets(line, sizeof(line), stdin) == NULL) continue;

        // Skip empty lines
        if (strcmp(line, "\n") == 0) continue;

        // Remove the newline character at the end
        line[strcspn(line, "\n")] = '\0';

        // Tokenize the input into command and arguments
        char *argv[21];  // Max 20 arguments plus NULL terminator
        int i = 0;
        char *token = strtok(line, " ");
        while (token != NULL && i < 20) {
            argv[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        argv[i] = NULL;  // NULL-terminate the argument array

        // Handle the built-in commands first
        if (argv[0] == NULL) continue;  // Skip empty input

        if (strcmp(argv[0], "exit") == 0) {
            // Exit the shell
            return 0;
        } else if (strcmp(argv[0], "help") == 0) {
            // Print the help message
            print_help();
        } else if (strcmp(argv[0], "cd") == 0) {
            // Handle 'cd' (change directory) built-in command
            handle_cd(argv);
        } else {
            // Handle external commands (commands in the allowed list)
            if (isAllowed(argv[0])) {
                execute_external_command(argv[0], argv);
            } else {
                // If the command is not allowed, print an error message
                printf("NOT ALLOWED!\n");
            }
        }
    }

    return 0;
}

// Function to execute an external command using posix_spawnp
void execute_external_command(char *cmd, char **args) {
    pid_t pid;
    int status;

    // Spawn the command
    if (posix_spawnp(&pid, cmd, NULL, NULL, args, environ) != 0) {
        perror("spawn failed");
        return;
    }

    // Wait for the command to finish
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
    }
}

// Function to handle the 'cd' command
void handle_cd(char **args) {
    if (args[1] == NULL) {
        // If no argument is provided, print an error
        printf("-rsh: cd: missing argument\n");
    } else if (args[2] != NULL) {
        // If there are too many arguments, print an error
        printf("-rsh: cd: too many arguments\n");
    } else if (chdir(args[1]) != 0) {
        // Try to change directory and print error on failure
        perror("cd failed");
    }
}

// Function to print the help message
void print_help() {
    printf("The allowed commands are:\n");
    for (int i = 0; i < N; i++) {
        printf("%d: %s\n", i + 1, allowed[i]);
    }
}
