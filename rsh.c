#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define N 12

extern char **environ;

//Array of elements allowed
char *allowed[N] = {"cp","touch","mkdir","ls","pwd","cat","grep","chmod","diff","cd","exit","help"};

//Checks if command is allowed
int isAllowed(const char* cmd) {
    for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            return 1;
        }
    }

    // If the command is not allowed, print NOT ALLOWED!
    return 0;
}

int main() {

    char line[256];

    //Main loop continuously prompts for user input
    while (1) {

        fprintf(stderr, "rsh>");

        //Read user input line
        if (fgets(line, 256, stdin) == NULL) continue;
        if (strcmp(line, "\n") == 0) continue;

        line[strlen(line) - 1] = '\0';

        //Tokenize and extract
        char *argv[21];
        int argc = 0;
        char *token = strtok(line, " ");
        while (token != NULL && argc < 20) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        if (argc == 0) continue; // Empty input

        //Check if command is allowed
        if (!isAllowed(argv[0])) {
            printf("NOT ALLOWED!\n");
            continue;
        }

        //Handle built in commands
        if (strcmp(argv[0], "cd") == 0) {
            if (argc > 2) {
                // Print the error message in the required format
                printf("-rsh: cd: too many arguments\n");
            } else if (argc == 2 && chdir(argv[1]) != 0) {
                perror("rsh cd failed");
            }
        } else if (strcmp(argv[0], "exit") == 0) {
            return 0;
        } else if (strcmp(argv[0], "help") == 0) {
            printf("The allowed commands are:\n");
            for (int i = 0; i < N; i++) {
                printf("%d: %s\n", i + 1, allowed[i]);
            }
        } else if (strcmp(argv[0], "mkdir") == 0) {
            if (argc < 2) {
                printf("rsh: mkdir: missing operand\n");
            } else {
                for (int i = 1; i < argc; i++) {
                    if (mkdir(argv[i], 0777) != 0) {
                        perror("rsh mkdir failed");
                    }
                }
            }
        } else if (strcmp(argv[0], "rmdir") == 0) {
            printf("NOT ALLOWED!\n");
        } else if (strcmp(argv[0], "touch") == 0) {
            if (argc < 2) {
                printf("rsh: touch: missing operand\n");
            } else {
                for (int i = 1; i < argc; i++) {
                    int fd = open(argv[i], O_CREAT | O_WRONLY, 0644);
                    if (fd == -1) {
                        perror("rsh touch failed");
                    } else {
                        close(fd);
                    }
                }
            }
        } else if (strcmp(argv[0], "ls") == 0) {
            //Handle ls command using posix_spawnp
            pid_t pid;
            int status;
            posix_spawnattr_t attr;
            posix_spawnattr_init(&attr);

            //Spawn new process to execute ls
            if (posix_spawnp(&pid, "ls", NULL, &attr, argv, environ) != 0) {
                perror("spawn failed :(");
            }

            // Wait for spawned process to finish
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid failed");
            }
        } else {
            // Handle all other allowed commands using posix_spawnp
            pid_t pid;
            int status;
            posix_spawnattr_t attr;
            posix_spawnattr_init(&attr);

            if (posix_spawnp(&pid, argv[0], NULL, &attr, argv, environ) != 0) {
                perror("spawn failed :(");
            }

            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid failed");
            }
        }

    }
    return 0;
}
