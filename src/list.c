Node *nodeNew(int dataSize, void *element)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->size = dataSize;
    node->data = malloc(dataSize);
    node->next = NULL;
    node->prev = NULL;

    memcpy(node->data, element, dataSize);

    return node;
}

void nodeGetValue(void *out, Node *node)
{
    memcpy(out, node->data, node->size);
}

List listNew(int typeSize)
{
    List list;

    list.typeSize = typeSize;
    list.maxSize = 0;
    list.count = 0;
    list.first = NULL;
    list.last = NULL;

    return list;
}

void listSetMaxSize(List *list, int size)
{
    list->maxSize = size;
}

void listClear(List *list)
{
    Node *current = list->first;

    while (list->first != NULL)
    {
        Node *toDelete = list->first;
        list->first = current->next;

        list->count--;

        free(toDelete);
    }

    list->last = NULL;
}

bool listAppend(List *list, void *element)
{
    if (list->maxSize >= list->count && list->maxSize > 0)
        return FALSE;

    Node *newNode = nodeNew(list->typeSize, element);

    if (list->count == 0)
    {
        list->first = newNode;
        list->last = newNode;

        list->count++;
        return TRUE;
    }

    list->last->next = newNode;
    newNode->prev = list->last;

    list->last = newNode;

    list->count++;
    return TRUE;
}

bool listPreppend(List *list, void *element)
{
    if (list->maxSize >= list->count && list->maxSize > 0)
        return FALSE;

    Node *newNode = nodeNew(sizeof(list->typeSize), element);

    if (list->count == 0)
    {
        list->first = newNode;
        list->last = newNode;

        list->count++;
        return TRUE;
    }

    list->first->prev = newNode;
    newNode->next = list->first;

    list->first = newNode;

    list->count++;
    return TRUE;
}

bool listHasElements(List *list)
{
    return list->count > 0;
}

Node *listPop(Node *node)
{
    if (node->next != NULL)
        node->next->prev = node->prev;

    if (node->prev != NULL)
        node->prev->next = node->next;

    return node;
}

Node *listSearchNode(List *list, void *data, EqualsCompair compair)
{
    Node *iterator = list->first;

    while (iterator != NULL)
    {

        if (compair(data, iterator->data))
            return iterator;

        iterator = iterator->next;
    }

    return NULL;
}
