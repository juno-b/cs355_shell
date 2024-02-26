//backgrounding shell
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include "List.h"
#include "Job.h"
#include "Tokenizer.h"

#define MYSH "mysh: "
#define DEBUG 1

//emacs -nw hw1.c&

List job_list; //holds a linked list of jobs
char **toks = NULL; //holds pointers to tokens
int num_jobs = 0; //holds the number of jobs
int command_fg = 0; //flag for foregrounding
int command_bg = 0; //flag for backgrounding
int command_resume = 0; //flag to resume a command in the foreground, 1 = bg
int last_bg_job = -1; //stores jobNum of last bg job, -1 if no background job
struct termios shell_tmodes; //stores terminal modes

//fn prototypes
void add_job(pid_t pid, char *com, int stat);
void handle_sigchld(int sig);
void ignore_me(int sig);
void ignore_signals();
void restore_signals();
void child_sigint_handler(int signo);
void blockSigchld();
void unblockSigchld();
void run_bg(char *com);
void to_fg(pid_t pid);
void execute_command(char *com);
int parse();
void free_toks();
int numCommands(char *line);
char **split_commands(char *line, int num_commands);
void free_commands(char **commands);

void add_job(pid_t pid, char *com, int stat) {
    add(&job_list, pid, com, stat);
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

void ignore_me(int sig) {
    //printf("\n%s", MYSH);
    fflush(stdout);
}

void ignore_signals() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    signal(SIGINT, &ignore_me);
    signal(SIGQUIT, &ignore_me);
    signal(SIGTSTP, &ignore_me);
    signal(SIGTTIN, &ignore_me);
    signal(SIGTTOU, &ignore_me);
    signal(SIGTERM, &ignore_me);
}

void restore_signals() {
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}

void child_sigint_handler(int signo) {
    if (signo == SIGINT) {
        printf("\n");
        fflush(stdout); // the new line is not printed
        exit(EXIT_FAILURE);
    }
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

void run_bg(char *com){
    pid_t pid = fork();
    if(pid < 0){
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){
        //Child process
        //set pgid to pid
        if(setpgid(0, getpid()) < 0){
            perror("Failed to set child's pgid (from child)");
            exit(EXIT_FAILURE);
        }
    }
    else{
        //add to job list
        add_job(pid, com, 1);
        last_bg_job = pid;
        //Parent process
        //set child's pgid to pid
        if(setpgid(pid, pid) < 0){
            perror("Failed to set child's pgid (from parent)");
            exit(EXIT_FAILURE);
        }
    }
}

void to_fg(int jn){
    if(DEBUG) printf("Foregrounding job %d\n", jn);
    if (jn < 1 || jn > num_jobs) {
        printf("fg: %%%d: no such job\n", jn);
        return;
    }
    //check if pid is in job list
    struct Job *job = get(&job_list, jn-1);
    if (job == NULL) {
        printf("fg: %%%d: no such job\n", jn);
        return;
    }
    //send SIGCONT to pid
    if(kill(job->pid, SIGCONT) < 0){
        perror("Failed to send SIGCONT");
        return;
    }
    //wait for pid to finish
    if(DEBUG) printf("Waiting for pid %d to finish\n", job->pid);
    //store terminal grp
    int term = tcgetpgrp(STDIN_FILENO);
    tcsetpgrp(STDIN_FILENO, job->pid); //give child control of terminal
    if (waitpid(job->pid, NULL, WUNTRACED) < 0) {
        perror("Failed to wait");
        exit(EXIT_FAILURE);
    }
    if(DEBUG) printf("Giving control back to shell\n");
    tcsetpgrp(STDIN_FILENO, term); //give control back to shell
    if (DEBUG) printf("Success\n");
    //remove from job list
    remove_job(&job_list, job->pid);
    num_jobs--;
}

void execute_command(char *com){
    if(command_bg == 1) run_bg(com);
    else{
        pid_t pid = fork();
        if(pid < 0){
            perror("Failed to fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            restore_signals();
            signal(SIGINT, child_sigint_handler);
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
            ignore_signals();
        }
        else{
            // Parent process
            // Add to job list
            add_job(pid, com, 0);
            last_bg_job = pid;
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
int parse(char *line){
    //store pointers in global array toks
    free_toks();
    int i = 0; int n = 0;
    add_history(line);
    TOKENIZER *t = init_tokenizer(line);
    //count tokens
    int flag = 1;
    int num_commands = 0;
    while(flag){
        char *tok = get_next_token(t);
        if(tok == NULL) flag = 0;
        else{
            if(strcmp(tok, "&") == 0){ 
                command_bg = 1; 
                //if(DEBUG) printf("Backgrounding command\n"); 
            }
            else n++;
            free(tok);
        }
    }
    //if (DEBUG) printf("Number of tokens: %d\n", n);
    //re-initialize tokenizer
    free_tokenizer(t);
    t = init_tokenizer(line);
    //allocate pointers to tokens
    toks = (char**) malloc((n+1) * sizeof(char*)); 
    if (toks == NULL) {
        perror("Failed to allocate memory for tokens");
        exit(EXIT_FAILURE);
    }  
    for (int i = 0; i < n+1; i++) {
        toks[i] = NULL;
    }   
    //store pointers to tokens
    i = 0;
    while(i < n){
        toks[i] = get_next_token(t);
        if(toks[i] == NULL) i = n;
        //else if (DEBUG) printf("Token %d: %s\n", i+1, toks[i]);
        i++;
    }
    //free tokenizer
    free_tokenizer(t);
    return n;
}

int numCommands(char *line) {
    int num_commands = 1; // At least one command exists
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == ';') {
            num_commands++;
        }
    }
    return num_commands;
}

char **split_commands(char *line, int num_commands) {
    char **commands = malloc(num_commands * sizeof(char *));
    if (commands == NULL) {
        perror("Failed to allocate memory for commands");
        exit(EXIT_FAILURE);
    }
    char *token;
    int i = 0;
    const char delim[] = ";";
    token = strtok(line, delim);
    while (token != NULL && i < num_commands) {
        commands[i] = strdup(token); // Allocate memory for each command
        if (commands[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, delim);
        i++;
    }
    return commands;
}

void free_toks(){
    if(toks == NULL) return;
    for(int i = 0; toks[i]!=NULL; i++){
        free(toks[i]);
    }
    free(toks);
}

void free_commands(char **commands) {
    if(commands == NULL) return;
    for (int i = 0; commands[i] != NULL; i++) {
        free(commands[i]);
    }
    free(commands);
}

int main(void) {
    ignore_signals();
    printf("Welcome to the backgrounding shell! Enter a command to get started.\n");
    printf("Type 'exit' to exit and 'history' to see previous processes.\n");
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
        toks = NULL;
        char * line = readline(MYSH);
        //handle ctrl+d
        if(line == NULL) continue;
        //handle newline
        if(strcmp(line, "") == 0){
            free(line);
            continue;
        }
        int num_commands = numCommands(line);
        char **commands = split_commands(line, num_commands);
        for(int current_command = 0; current_command < num_commands; current_command++){
            //if(line != NULL) free(line);
            line = commands[current_command];
            int num_toks = parse(line);
            if(num_toks == 0) continue;
            if(toks[0] == NULL) {
                printf("No command stored.\n");
                exit(EXIT_FAILURE);
            }
            //compare user input to "exit"
            if (strcmp(toks[0], "exit") == 0) {
                //free input, job_list and clear history     
                clear(&job_list);
                free_commands(commands);
                if(line != NULL) free(line);
                free_toks();
                clear_history();
                //exit
                printf("Goodbye!\n");
                exit(0);
            }
            if(strcmp(toks[0], "history") == 0){        
                HIST_ENTRY **hist_list = history_list();     
                for (int i = 0; hist_list[i] != NULL; i++){  
                    printf("%d: %s\n", i + history_base, hist_list[i]->line);
                }
                continue;
            }
            else if(strcmp(toks[0], "jobs") == 0){
                print(&job_list);
                continue;
            }
            else if(strcmp(toks[0], "fg") == 0){
                if(toks[1] == NULL || strcmp(toks[1], "%") != 0 || toks[2] == NULL){
                    printf("Job incorrectly specified.\n");
                }
                else{
                    // Extracting job ID from toks[1]
                    int job_id = atoi(toks[2]); 
                    if (job_id >=1){      
                        struct Job *job = get(&job_list, job_id-1);      
                        if (job != NULL) to_fg(job_id);
                        else {
                            printf("%s: %s: Job not found.\n", toks[0], toks[1]);
                        }
                    }
                    else{
                        printf("%s: %s: Job not found.\n", toks[0], toks[1]);
                    }
                }
                continue;
            }
            else if(strcmp(toks[0], "bg") == 0){
                if(toks[1] == NULL || strcmp(toks[1], "%") != 0 || toks[2] == NULL){
                    printf("Job incorrectly specified.\n");
                }
                else{
                    // Extracting job ID from toks[1]
                    int job_id = atoi(toks[2]); 
                    if (job_id >=1){      
                        struct Job *job = get(&job_list, job_id-1);      
                        if (job != NULL) to_fg(job_id);
                        else {
                            printf("%s: %s: Job not found.\n", toks[0], toks[1]);
                        }
                    }
                    else{
                        printf("%s: %s: Job not found.\n", toks[0], toks[1]);
                    }
                }
                continue;
            }
            else if(strcmp(toks[0], "kill") == 0){
                if(toks[1] == NULL || toks[2] == NULL){
                    printf("No job specified.\n");
                }
                //toks[1] can either be %jobNum or -9
                else if(strcmp(toks[1], "-9") == 0){
                    if(toks[2] == NULL || strcmp(toks[2], "%") != 0 || toks[3] == NULL){
                        printf("No job specified.\n");
                    }
                    else{
                        // Extracting job ID from toks[3]
                        int job_id = atoi(toks[3]); 
                        if (job_id >=1){      
                            struct Job *job = get(&job_list, job_id-1);      
                            if (job != NULL){
                                if(kill(job->pid, SIGKILL) < 0){
                                    perror("Failed to send SIGKILL");
                                }
                            }
                            else {
                                printf("%s %s %s%s: Job not found.\n", toks[0], toks[1], toks[2], toks[3]);   
                            }
                        }
                        else {
                            printf("%s %s %s%s: Job not found.\n", toks[0], toks[1], toks[2], toks[3]);   
                        }
                    }
                }
                else if(strcmp(toks[1], "%") == 0){
                    // Extracting job ID from toks[2]
                    int job_id = atoi(toks[2]); 
                    if (job_id >=1){      
                        struct Job *job = get(&job_list, job_id-1); 
                        if (job != NULL){
                            if(kill(job->pid, SIGTERM) < 0){
                                perror("Failed to send SIGTERM");
                            }
                        }
                        else {
                            printf("%s: %s%s: Job not found.\n", toks[0], toks[1], toks[2]);    
                        }
                    }
                    else{
                        printf("%s: %s%s: Job not found.\n", toks[0], toks[1], toks[2]);
                    }
                }
                else{
                    printf("Invalid job specified.\n");
                }
                continue;
            }
            else if(strcmp(toks[0], "cd") == 0){
                if(toks[1] == NULL){
                    printf("No directory specified.\n");
                    
                }
                else if(chdir(toks[1]) < 0){
                    perror("Failed to change directory");
                }
                continue;
            }
            blockSigchld();
            if(DEBUG) printf("Executing command...\n");
            execute_command(line);
            if(DEBUG) printf("Command executed.\n");
            unblockSigchld();  
        }
    }
    return EXIT_SUCCESS;
}
