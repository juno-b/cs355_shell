#include <stdlib.h>
#include "Job.h"

struct Job *newJob(pid_t id) { // Creates a new node with item
    struct Job *n = malloc(sizeof(struct Job));
    pid_t pid = id;
    //char *command = command;
    //int job_number;
    //int status; // 0: running, 1: stopped, 2: done
    struct Job *next = NULL;
} // newNode()