#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include "Tokenizer.h"

#define MAX_ARGS 64
#define PROMPT "mysh> "
int running = 1;
char **toks; // holds pointers to tokens
int command_bg = 0; // flag for backgrounding

void execute_command();
void child_sigint_handler(int signo);
int parse(char *line);
void free_toks();

int main() {
    signal(SIGINT, SIG_IGN);

    char *input;

    while (running) {
        input = readline(PROMPT); // print the prompt and get command

        // check null and empty string
        if (!input) {
            printf("\n");
            break;
        }
        if(strcmp(input, "") == 0){
            free(input);
            break;
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

        blockSigchld();
        execute_command();
        unblockSigchld();
        free_toks();


        free(input);
    }

    return 0;
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
