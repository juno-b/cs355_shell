#include <stdlib.h>
#include "Job.h"

struct Job *newJob(pid_t pid, char *command, int status) { // Creates a new node with item
    struct Job *new = malloc(sizeof(struct Job));
    new -> pid = pid;
    new -> command = command;
    new -> status = status; // 0: running, 1: stopped, 2: done
    new -> next = NULL;
    return new;
} // newJob()

// i am not sure but i think you didnt actually assign the args to the fields?
