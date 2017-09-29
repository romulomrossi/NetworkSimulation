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

void addGraphEdge(Graph graph, int idOrigin, int idDestiny, int cost)
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
        listAppend(graph, &newNode);
    }
}

GraphNode *getGraphNode(Graph graph, int id)
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

List initializeDijkstra(Graph graph, int originNode)
{
    ListNode *current = graph->first;
    List response = listNew(sizeof(DijkstraResponse));
    GraphNode *currentGraphNode;

    while (current != NULL)
    {
        currentGraphNode = (GraphNode *)current->data;
        DijkstraResponse newResponse;
        newResponse.destinyNode = currentGraphNode->id;
        newResponse.cost = currentGraphNode->id == originNode ? 0 : INF;
        newResponse.path = listNew(sizeof(int));
        newResponse.prevNode = -1;
        newResponse.done = FALSE;

        listAppend(&response, &newResponse);

        current = current->next;
    }

    return response;
}

List runDijkstra(Graph graph, int originNode)
{
    List response = initializeDijkstra(graph, originNode);
    DijkstraResponse *currentResponse = decideNext(&response);
    GraphNode *currentGraphNode = getGraphNode(graph, currentResponse->destinyNode);

    while (currentGraphNode != NULL && currentResponse != NULL)
    {
        relaxEdges(&response, currentGraphNode, currentResponse);

        currentResponse = decideNext(&response);
        if (currentResponse != NULL)
            currentGraphNode = getGraphNode(graph, currentResponse->destinyNode);
    }
    return response;
}

bool DijkstraCompair(void *a, void *b)
{
    return *((int *)a) == ((DijkstraResponse *)b)->destinyNode;
}

void relaxEdges(Dijkstra dijkstra, GraphNode *graphNode, DijkstraResponse *dijkstraPrev)
{
    ListNode *currentEdge = graphNode->adjacency.first;
    GraphEdge *currentEdgeData;
    int newCost = INF;
    DijkstraResponse *destinyDijkstra;
    while (currentEdge != NULL)
    {
        currentEdgeData = (GraphEdge *)currentEdge->data;
        newCost = dijkstraPrev->cost + currentEdgeData->cost;

        destinyDijkstra = (DijkstraResponse *)listSearchNode(dijkstra, &currentEdgeData->destiny, &DijkstraCompair)->data;
        if (destinyDijkstra->cost > newCost)
        {
            destinyDijkstra->cost = newCost;
            destinyDijkstra->prevNode = graphNode->id;
        }

        currentEdge = currentEdge->next;
    }

    dijkstraPrev->done = TRUE;
}

DijkstraResponse *decideNext(Dijkstra dijkstra)
{
    ListNode *current = dijkstra->first;
    int bestCost = INF;
    DijkstraResponse *best = NULL;
    DijkstraResponse *currentData;

    while (current != NULL)
    {
        currentData = (DijkstraResponse *)current->data;

        if (currentData->done)
        {
            current = current->next;
            continue;
        }

        if (currentData->cost < bestCost)
        {
            bestCost = currentData->cost;
            best = currentData;
        }

        current = current->next;
    }

    return best;
}

void dijkstraBacktrack(Dijkstra dijkstra)
{
    ListNode * currentNode = dijkstra->first;
    DijkstraResponse * currentData;
    while (currentNode != NULL)
    {
        currentData = (DijkstraResponse *) currentNode->data;

        

        currentNode = currentNode->next;
    }
}
