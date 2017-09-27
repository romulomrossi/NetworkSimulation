typedef Node ListNode;

List newGraph()
{
    List graph = listNew(sizeof(GraphNode));

    return graph;
}

GraphNode newGraphNode(int id)
{
    GraphNode node;

    node.id = id;
    node.adjacency = listNew(sizeof(GraphEdge));

    return node;
}

void addGraphEdge(List *graph, int idOrigin, int idDestiny, int cost)
{
    GraphNode *node = getGraphNode(graph, idOrigin);
    GraphEdge edge;

    edge.destiny = idDestiny;
    edge.cost = cost;

    if (node == NULL)
    {
        GraphNode newNode = newGraphNode(idOrigin);
        listAppend(&newNode.adjacency, &edge);
        listAppend(graph, &newNode);
    }
    else
        listAppend(&node->adjacency, &edge);

    GraphNode *destinyNode = getGraphNode(graph, idDestiny);

    if (destinyNode == NULL)
    {
        GraphNode newNode = newGraphNode(idDestiny);
        listAppend(&newNode.adjacency, &edge);
        listAppend(graph, &newNode);
    }
}

GraphNode *getGraphNode(List *graph, int id)
{
    if (graph->count == 0)
        return NULL;

    ListNode *nodeList = graph->first;
    while (nodeList != NULL)
    {
        GraphNode *nodeGraph = (GraphNode *)nodeList->data;

        if (nodeGraph->id == id)
            return nodeGraph;

        nodeList = nodeList->next;
    }

    return NULL;
}

