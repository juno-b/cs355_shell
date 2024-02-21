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
#include "Tokenizer.h"

#define MAX_ARGS 10
#define MYSH "mysh: "
#define DEBUG 1

//emacs -nw hw1.c&

List job_list; //holds a linked list of jobs
char **toks; //holds pointers to tokens
int num_jobs = 0; //holds the number of jobs
int command_fg = 0; //flag for foregrounding
int command_bg = 0; //flag for backgrounding
int command_resume = 0; //flag to resume a command in the foreground, 1 = bg
pid_t last_bg_job = -1; //stores pid of last bg job, -1 if no background job

//fn prototypes
void add_job(pid_t pid);
void handle_sigchld(int sig);
void handle_sigstop(int sig);
void blockSigchld();
void unblockSigchld();
void run_bg();
void to_fg(pid_t pid);
void execute_command();
int parse();
void free_toks();

void add_job(pid_t pid) {
    add(&job_list, pid);
    num_jobs++;
}

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

void handle_sigstop(int sig) {
    printf("Child process has stopped.\n");
}

void blockSigchld() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

void unblockSigchld() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void run_bg(){
    pid_t pid = fork();
    //add to job list
    add_job(pid);
    last_bg_job = pid;
    if(pid < 0){
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){
        //execute command and arguments
        int errno_exec = execvp(toks[0], toks);
        //check for ENOENT
        if(errno_exec == -1){
            printf("%s: Command not found\n", toks[0]);  
            exit(EXIT_FAILURE);
        }
        else if(errno_exec < 0){
            perror("Failed to execute command");
            exit(EXIT_FAILURE);
        }
    }
}

void to_fg(pid_t pid){
    //check if pid is in job list
    if(!contains(&job_list, pid)){
        printf("Job %d not found.\n", pid);
        return;
    }
    //send SIGCONT to pid
    if(kill(pid, SIGCONT) < 0){
        perror("Failed to send SIGCONT");
        return;
    }
    //wait for pid to finish
    tcsetpgrp(STDIN_FILENO, pid); //give child control of terminal
    if (waitpid(pid, NULL, WUNTRACED) < 0) {
        perror("Failed to wait");
        exit(EXIT_FAILURE);
    }
    tcsetpgrp(STDIN_FILENO, getpgrp()); //give control back to shell
    //remove from job list
    remove_job(&job_list, pid);
    num_jobs--;
}

void execute_command(){
    if(command_bg == 1) run_bg();
    else if(last_bg_job != -1) to_fg(last_bg_job);
    else{
        pid_t pid = fork();
        //add to job list
        add_job(pid);
        last_bg_job = pid;
        if(pid < 0){
            perror("Failed to fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            //execute command and arguments
            int errno_exec = execvp(toks[0], toks);
            //check for ENOENT
            if(errno_exec == -1){
                printf("%s: Command not found\n", toks[0]);  
                exit(EXIT_FAILURE);
            }
            else if(errno_exec < 0){
                perror("Failed to execute command");
                exit(EXIT_FAILURE);
            }
        }
        else{
            // Parent process
            // Wait for foreground process to finish
            int status;
            if (waitpid(pid, &status, WUNTRACED) < 0) {
                perror("Failed to wait");
                exit(EXIT_FAILURE);
            }
            // Check if the foreground process was stopped by a signal
            if (WIFSTOPPED(status)) {
                printf("Foreground process stopped.\n");
            }
            // Remove the foreground process from the job list
            remove_job(&job_list, pid);
            num_jobs--;
        }
    }
}

//reads a line and returns # of tokens
int parse(){
    //store pointers in global array toks
    int i = 0; int n = 0;
    char * line = readline(MYSH);
    //handle ctrl+d
    if(line == NULL) return 0;
    //handle newline
    if(strcmp(line, "") == 0){
        if(line != NULL) free(line);
        return 0;
    }
    if (DEBUG) printf("Command entered: %s\n", line);
    add_history(line);
    TOKENIZER *t = init_tokenizer(line);
    //count tokens
    int flag = 1;
    while(flag){
        char *tok = get_next_token(t);
        if(tok == NULL) flag = 0;
        else{
            free(tok);
            n++;
        }
    }
    while(get_next_token(t) != NULL) n++;
    if (DEBUG) printf("Number of tokens: %d\n", n);
    //re-initialize tokenizer
    free_tokenizer(t);
    t = init_tokenizer(line);
    //allocate pointers to tokens
    toks = (char**) malloc((n+1) * sizeof(char*));       
    //store pointers to tokens
    flag = 1;
    while(flag){
        toks[i] = get_next_token(t);
        if(toks[i] == NULL) flag = 0;
        else if (DEBUG) printf("Token %d: %s\n", i, toks[i]);
        i++;
    }
    //while((toks[i++] = get_next_token(t)) != NULL);      
    //if last token is &, set bg flag
    if(toks[n-1] != NULL && strcmp(toks[n-1], "&") == 0){
        if(DEBUG) printf("Backgrounding command\n");
        command_bg = 1;
    }
    //free tokenizer
    free_tokenizer(t);
    if(line != NULL) free(line);
    return n;
}

void free_toks(){
    for(int i = 0; toks[i]!=NULL; i++){
        free(toks[i]);
    }
    free(toks);
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

    //loop for shell functionality
    while(1) {
        int num_toks = parse();
        if(num_toks == 0) continue;
        if(toks[0] == NULL) {
            printf("No command stored.\n");
            exit(EXIT_FAILURE);
        }
        //compare user input to "exit"
        if (strcmp(toks[0], "exit") == 0) {
            //free input, job_list and clear history     
            clear(&job_list);
            free_toks();
            clear_history();
            //exit
            printf("Goodbye!\n");
            exit(0);
        }
        int input_hist_flag = 1;
        int cont_flag = 0;
        while(input_hist_flag){

            //add print statements for usage
            if (strcmp(toks[0], "help") == 0) {
                printf("Exit: exit\n");
                printf("This terminal supports history using the following commands:\n");
                printf("Repeat the last line of input: !!\n");
                printf("Repeat the nth command: !n\n");      
                printf("Repeat the nth most recent command: !-n\n");
                cont_flag = 1;
                input_hist_flag = 0;
            }

            //handle history commands
            else if(strcmp(toks[0], "history") == 0){        
                HIST_ENTRY **hist_list = history_list();     
                for (int i = 0; hist_list[i] != NULL; i++){  

                    printf("%d: %s\n", i + history_base, hist_list[i]->line);
                }
                cont_flag = 1;
                input_hist_flag = 0;
            }
            else if (strcmp(toks[0], "!!") == 0) {
                HIST_ENTRY *last_entry = history_get(history_length - 1);
                if (last_entry != NULL) {
                    strcpy(toks[0],last_entry->line);        
                }
                else {
                    printf("No previous command found!\n");  

                    cont_flag = 1;
                    input_hist_flag = 0;
                }
            }
            else if (strcmp(toks[0], "!") == 0) {
                int n;
                if (sscanf(toks[1], "%d", &n) == 1) {        
                    //!n and !-n
                    if (n < 0) n = history_length + n;       
                    HIST_ENTRY *entry = history_get(n);      
                    if (entry != NULL) {
                        strcpy(toks[0],entry->line);
                    }
                    else {
                        printf("%s: Command not found.\n", toks[0]);
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
        blockSigchld();
        if(DEBUG) printf("Executing command...\n");
        execute_command();
        if(DEBUG) printf("Command executed.\n");
        unblockSigchld();
        free_toks();
    }
    return EXIT_SUCCESS;
}
