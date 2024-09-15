#ifndef lcthw_List_h
#define lcthw_List_h

#include <stdlib.h>
#include <stdio.h>


#define check_mem(ptr) if(ptr == NULL) { printf("Memory allocation failed.\n"); exit(1); }
#define check(cond, msg) if(!(cond)) { printf("Error: %s\n", msg); exit(1); }

#define mu_run_test(test) do { char *msg = test(); \
    if (msg) { printf("FAILED: %s\n", msg); return (void*)0; } } while (0)

#define mu_assert(test, msg) do { if (!(test)) { \
    printf("FAILED: %s:%d: %s\n", __FILE__, __LINE__, msg); \
    return (void*)0; } } while (0)

#define mu_suite_start() printf("Running tests:\n")

#define debug(msg, ...) printf("DEBUG %s:%d: " msg, __FILE__, __LINE__, ##__VA_ARGS__)

struct ListNode;

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *value;
} ListNode;

typedef struct List {
    int count;
    ListNode *first;
    ListNode *last;
} List;

List *List_create();
void List_destroy(List *list);
void List_clear(List *list);
void List_clear_destroy(List *list);

#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)

void List_push(List *list, void *value);
void *List_pop(List *list);

void List_unshift(List *list, void *value);
void *List_shift(List *list);

void *List_remove(List *list, ListNode *node);

#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
    ListNode *V = NULL;\
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif
