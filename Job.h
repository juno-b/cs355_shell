#ifndef JOB_H
#define JOB_H

typedef struct Job {
    pid_t pid;
    //char *command;
    //int job_number;
    //int status; // 0: running, 1: stopped, 2: done
    struct Job *next;
} Job;

struct Job *newJob(pid_t id);//, char *command, int job_number, int status);

#endif