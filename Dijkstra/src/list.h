/*
    Author: Rômulo Rossi
    Email: romulomrossi@hotmail.com
*/

typedef enum {FALSE, TRUE} bool;

typedef bool (*EqualsCompair)(void *, void *);

typedef struct s_Node{
    void *data;
    long size;
    struct s_Node *next;
    struct s_Node *prev;
}Node;

typedef struct s_List{
    int typeSize;
    int count;
    int maxSize;
    Node *first;
    Node *last;
}List;

Node* nodeNew(int dataSize, void *element);
void nodeGetValue(void *out, Node *node);

List listNew(int typeSize);
void listClear(List *list);

bool listAppend(List *list, void *element);
bool listPrepend(List *list, void *element);

Node *listPop(List *list, Node *node);
Node *listSearchNode(List *list, void *data, EqualsCompair compair);

void listSetMaxSize(List * list, int size);

bool listHasElements(List *list);

#include "list.c"