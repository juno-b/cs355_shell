#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>

#define MAX_ARGS 64
#define PROMPT "mysh> "
int running = 1;


void execute_command(char *command);
void child_sigint_handler(int signo);

int main() {
    signal(SIGINT, SIG_IGN);

    char *input;

    while (running) {
        input = readline(PROMPT); // print the prompt and get command
        if (!input) {
            printf("\n");
            break;
        }

        char *trimmed_input = strtok(input, "\n");
        if (trimmed_input) {
            execute_command(trimmed_input);
        }

        free(input);
    }

    return 0;
}

void child_sigint_handler(int signo) {
    if (signo == SIGINT) {
        printf("\n");
        fflush(stdout); // the new line is not printed
        exit(EXIT_FAILURE);
    }
}

void execute_command(char *command) {
    char *args[MAX_ARGS];
    
    int i = 0;
    char *token = strtok(command, " \t\n&;|><");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i] = token;
        token = strtok(NULL, " \t\n&;|><");
        i++;
    }
    args[i] = NULL; // null terminated

    // built-in command: exit
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    // fork a new process to execute the command
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) { // child process
        signal(SIGINT, child_sigint_handler);
        if (execvp(args[0], args) < 0) {
            perror("command not found");
            exit(1);
        }
    } else { // parent process
        int status;
        while (wait(&status) != pid) {
            if (status == -1) {
                perror("wait failed");
                break;
            }
        }
    }
}