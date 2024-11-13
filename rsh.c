#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12

extern char **environ;

char *allowed[N] = {"cp","touch","mkdir","ls","pwd","cat","grep","chmod","diff","cd","exit","help"};

int isAllowed(const char* cmd) {
    // TODO
    // return 1 if cmd is one of the allowed commands
    // return 0 otherwise
    for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            //if the two strings match using string compare, return 1(true)
            return 1;
        }
    }
    
    return 0;
}

int main() {

    // TODO
    // Add variables as needed

    char line[256];

    while (1) {

        fprintf(stderr, "rsh>");

        if (fgets(line, 256, stdin) == NULL) continue;

        if (strcmp(line, "\n") == 0) continue;

        line[strlen(line) - 1] = '\0';

        // TODO
        // Add code to spawn processes for the first 9 commands
        // And add code to execute cd, exit, help commands
        // Use the example provided in myspawn.c
        char *argv[21];
        int argc = 0;
        char *token = strtok(line, " ");
        while (token != NULL && argc < 20) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        if (argc == 0) continue; //empty input

        if (!isAllowed(argv[0])) {
            //enters this if statement if a 0 is returned from the isAllowed function
            printf("not allowed \n");
            continue;
        }
        
        if (strcmp(argv[0], "cd") == 0) {
            if (argc > 2) {
                printf("rsh: too many arguments for cd");
            } else if (argc == 2 && chdir(argv[1]) != 0) {
                perror("rsh cd failed");
            }
        } else if (strcmp(argv[0], "exit") == 0) {
            return 0;
        } else if (strcmp(argv[0], "help") == 0) {
            printf("The allowed commands are: \n");
            for (int i = 0; i < N; i++) {
                printf("%s\n", allowed[i]);
            }
        } else {
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

