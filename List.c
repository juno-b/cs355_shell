#include <stdlib.h>
#include <stdio.h>
#include "Job.h"
#include "List.h"

List newList() { // Creates a new empty list
    List *L = malloc(sizeof(List));
    L->head = NULL;
    L->tail = NULL;
    L->size = 0;
    return *L;
} // newList()

int size(const List *l){ // Returns the size of list-l
    return l->size;
} // size()

int empty(const List *l){ // is the list-l empty?
    return l->size == 0;
} // empty()

void clear(List *l) { // removes all items from list-l
    struct Job *n = l->head;
    struct Job *nxt;
    while (n != NULL) { // Visit each Job and recycle it
        nxt = n->next;
        free(n);
        n = nxt;
    }
    l->head = l->tail = NULL; // All recycled! Now reset.
    l->size = 0;
} // clear()

void add(List *l, pid_t item){ // Add item at end of list-l
    struct Job *n = malloc(sizeof(struct Job));
    n->pid = item;
    n->next = NULL;
    if (l->head == NULL) {
        l->head = n;
        l->tail = n;
    } else {
        l->tail->next = n;
        l->tail = n;
    }
    l->size++;
} // add()

void remove_job(List *l, pid_t item){ // Remove item from list-l
    struct Job *n = l->head;
    struct Job *prev = NULL;
    while (n != NULL) {
        if (n->pid == item) {
            if (prev == NULL) {
                l->head = n->next;
            } else {
                prev->next = n->next;
            }
            if (n->next == NULL) {
                l->tail = prev;
            }
            free(n);
            l->size--;
            return;
        }
        prev = n;
        n = n->next;
    }
    printf("Error: Item not found in list. Exiting!\n");
    exit(EXIT_FAILURE);
} // remove()

struct Job *get(const List *l, int index){ // Returns item at index in list-l
    if (index < 0 || index >= l->size) {
        printf("Error: List index out of bounds %d. Exiting!\n", index);
        exit(EXIT_FAILURE);
    }
    // index is valid, lets walk…
    struct Job *n=l->head; // start at head
    for (int i=0; i < index; i++)
        n = n->next; // hop!
    return n; // we’re there!
} // get()

int contains(const List *l, struct Job item){ // Does list-l have item?
    struct Job *n = l->head;
    pid_t pid = item.pid;
    while (n != NULL) {
        if (n->pid == pid) {
            return 1;
        }
        n = n->next;
    }
    return 0;
} // contains()

void print(const List *l){ // prints contents of list (test only)
    struct Job *n = l->head;
    while (n != NULL) {
        printf("%d ", n->pid);
        n = n->next;
    }
    printf("\n");
} // print()