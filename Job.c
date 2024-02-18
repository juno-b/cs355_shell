#include <stdlib.h>
#include "Job.h"

struct Job *newJob(pid_t id, char *com, int stat) { // Creates a new node with item
    struct Job *n = malloc(sizeof(struct Job));
    pid_t pid = id;
    char *command = com;
    int status = stat; // 0: running, 1: stopped, 2: done
    struct Job *next = NULL;
} // newNode()