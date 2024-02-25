#ifndef LIST_H
#define LIST_H

//struct for a List
typedef struct {
    int size;
    struct Job *head;
    struct Job * tail;
} List;

//Fn prototypes for List
List newList(); // Creates a new empty list
int size(const List *l); // Returns the size of list-l
int empty(const List *l); // is the list-l empty?
void clear(List *l); // removes all items from list-l
void add(List *l, pid_t item, char *com, int stat); // Add item at end of list-l
void remove_job(List *l, pid_t item); // Remove item from list-l
struct Job *get(const List *l, int index); // Returns item at index in list-l
int contains(const List *l, pid_t pid); // Does list-l have item?
void print(const List *l); // prints contents of list 

#endif
