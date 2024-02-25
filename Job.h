#ifndef JOB_H
#define JOB_H

typedef struct Job {
    pid_t pid;
    char *command;
    int status; // 0: running, 1: stopped, 2: done
    int jobNum;
    struct Job *next;
} Job;

struct Job *newJob(pid_t id, char *com, int stat);

#endif