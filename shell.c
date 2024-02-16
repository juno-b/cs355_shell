//backgrounding shell
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <signal.h>
#include "List.h"
#include "Job.h"

#define MAX_ARGS 10

//emacs -nw hw1.c&

List job_list;
int num_jobs = 0;

void handle_sigchld(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)){
            printf("Child process %d has terminated.\n", pid);
            //remove from job list
            remove_job(&job_list, pid);
            num_jobs--;
        }
    }
}

void add_job(pid_t pid) {
    add(&job_list, pid);
    num_jobs++;
}

int main(void) {
    printf("Welcome to the backgrounding shell! Enter a command to get started.\n");
    printf("Type 'exit' to exit or 'help' for more information\n");
    //Set up signal handler for SIGCHLD
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error setting up signal handler for SIGCHLD");
        exit(EXIT_FAILURE);
    }

    //add support for history
    int hist_size = 50;
    char *hist_size_str = getenv("HISTSIZE");
    if(hist_size_str != NULL){
        hist_size = atoi(hist_size_str);
    }
    using_history();
    stifle_history(hist_size);

    //initialize job list
    job_list = newList();

    //loop for shell functionality
    while(1) {
        //read user input
        char *input = NULL;
        input = readline("mysh: ");
        //printf("INPUT: %s\n", input);
        if (input == NULL) {
            perror("Failed to read line");
            exit(EXIT_FAILURE);
        }

        //add the command to history
        add_history(input);
        //compare user input to "exit"
        if (strcmp(input, "exit") == 0) {
            //free input, job_list and clear history
            free(input);
            clear(&job_list);
            //free(job_list);
            clear_history();
            //exit
            printf("Goodbye!\n");
            exit(0);
        }
        int input_hist_flag = 1;
        int cont_flag = 0;
        while(input_hist_flag){

        //add print statements for usage
        if (strcmp(input, "help") == 0) {
            printf("Exit: exit\n");
            printf("This terminal supports history using the following commands:\n");
            printf("Repeat the last line of input: !!\n");
            printf("Repeat the nth command: !n\n");
            printf("Repeat the nth most recent command: !-n\n");
            //free(input);
            cont_flag = 1;
            input_hist_flag = 0;
        }
        
        //handle history commands
        else if(strcmp(input, "history") == 0){
            HIST_ENTRY **hist_list = history_list();
            for (int i = 0; hist_list[i] != NULL; i++){        
                printf("%d: %s\n", i + history_base, hist_list[i]->line);
            }
            //free(input);
            cont_flag = 1;
            input_hist_flag = 0;
        }
        else if (strcmp(input, "!!") == 0) {  
            HIST_ENTRY *last_entry = history_get(history_length - 1);
            if (last_entry != NULL) {
                strcpy(input,last_entry->line);
            } 
            else {
                printf("No previous command found!\n");
                //free(input);
                cont_flag = 1;
                input_hist_flag = 0;
            }
        } 
        else if (input[0] == '!') {
            int n;
            if (sscanf(input + 1, "%d", &n) == 1) {
                //!n and !-n
                if (n < 0) n = history_length + n;
                HIST_ENTRY *entry = history_get(n);
                if (entry != NULL) {
                    strcpy(input,entry->line);
                }
                else {
                    printf("Command not found.\n");
                    //free(input);
                    cont_flag = 1;
                    input_hist_flag = 0;
                }
            } 
        }
        else{
            input_hist_flag = 0;
        }

        }
        if(cont_flag) continue;

        //parse commands and arguments
        //handle commands such as emacs, code, ps, cat, gcc, ls, pwd, sleep
        char *command = strtok(input, " &;|<>\n");
        //printf("command: %s\n", command);

        char *args[MAX_ARGS + 1];
        args[0] = command;
        int arg_count = 1;

        //parse and store arguments
        char *arg = strtok(NULL, " &;|<>\n");
        while (arg != NULL && arg_count < MAX_ARGS) {
            args[arg_count++] = arg;
            //printf("arg%d: %s\n", arg_count-1, args[arg_count-1]);
            arg = strtok(NULL, " &;|<>\n");
        }
        args[arg_count] = NULL;
        
        //check if command is a background process
        int background = 0;
        if (arg_count > 1 && strcmp(args[arg_count - 1], "&") == 0) {
            background = 1;
            args[arg_count - 1] = NULL;
        }

        //fork a child to execute command and arguments
        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            //execute command and arguments
            if (execvp(command, args) < 0) {
                perror("Failed to execute command");
                exit(EXIT_FAILURE);
            }
        }
        else {
            if (!background) {
                //wait for child to finish
                tcsetpgrp(STDIN_FILENO, pid); //give child control of terminal
                if (wait(NULL) < 0) {
                    perror("Failed to wait");
                    exit(EXIT_FAILURE);
                }
                tcsetpgrp(STDIN_FILENO, getpgrp()); //give control back to shell
            }
            else {
                //add to job list
                add_job(pid);
            }
        }
        
        //free input
        if (input != NULL) free(input);
    }
    return 0;
}
