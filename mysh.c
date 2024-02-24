#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include "Job.h"
#include "List.h"
#include "Tokenizer.h"

#define MAX_ARGS 64
#define PROMPT "mysh> "


// global variables
int running = 1;
List jobs;
int num_jobs = 0;
char **toks; // pointers to tokens
int command_bg = 0; // whether the command is backgrounding
int command_fg = 0; // whether the command is foregrounding
int command_resume = 0; // whether the command is to resume a command in the foreground, 1 = bg
pid_t last_bg_job = -1; // stores the last bg job, -1 represents no bg job

// prototypes
void execute_command();
void child_sigint_handler(int signo);
void blockSigchld();
void unblockSigchld();
void run_bg(); // run a process at background
void to_fg(pid_t pid); // bring a process to foreground
int parse(char *line);
void free_toks();

int main() {
    signal(SIGINT, SIG_IGN);

    char *input;

    while (running) {
        input = readline(PROMPT); // print the prompt and get command

        if (!input) { // handle null input
            printf("\n");
            break;
        }

        if (strcmp(input, "") == 0) { // handle empty string
            free(input);
            continue;
        }

        int num_toks = parse(input);

        if(num_toks == 0) {
            continue;
        }

        if(toks[0] == NULL) {
            printf("No command stored.\n");
            exit(EXIT_FAILURE);
        }

        if(toks[0] == NULL) {
            printf("No command stored.\n");
            exit(EXIT_FAILURE);
        }

        //blockSigchld();
        execute_command();
        //unblockSigchld();
        free_toks();
    }

    return EXIT_SUCCESS;
}


void free_toks(){
    for(int i = 0; toks[i]!=NULL; i++){
        free(toks[i]);
    }
    free(toks);
}

void child_sigint_handler(int signo) {
    if (signo == SIGINT) {
        printf("\n");
        fflush(stdout); // the new line is not printed
        exit(EXIT_FAILURE);
    }
}

void execute_command() {

    // built-in command: exit
    if (strcmp(toks[0], "exit") == 0) {
        exit(0);
    }

    // built-in command: jobs
    if (strcmp(toks[0], "jobs") == 0) {
        print(&jobs);
        return;
    }

    // fork a new process to execute the command
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    } else if (pid == 0) { // child process
        signal(SIGINT, child_sigint_handler);
        if (execvp(toks[0], toks) < 0) {
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


int parse(char *input){ // return number of tokens
    int i = 0;
    int n = 0; // number of tokens
    Tokenizer *tokenizer = init_tokenizer(input);

    int flag = 1;
    while(flag){
        char *tok = get_next_token(tokenizer);
        if(tok == NULL) {
            flag = 0;
        } else {
            free(tok);
            n++;
        }
    }

    //re-initialize tokenizer
    free_tokenizer(tokenizer);
    tokenizer = init_tokenizer(input);
    //allocate pointers to tokens
    toks = (char**) malloc((n+1) * sizeof(char*));       
    //store pointers to tokens
    flag = 1;
    while(flag){
        toks[i] = get_next_token(tokenizer);
        if(toks[i] == NULL) {
            flag = 0;
        }
        i++;
    }

    // if last token is &
    if(toks[n-1] != NULL && strcmp(toks[n-1], "&") == 0){
        command_bg = 1;
    }

    free_tokenizer(tokenizer);
    if(input != NULL) {
        free(input);
    }
    return n;
}
