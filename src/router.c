/*
    Author: Rômulo Rossi
    Email: romulomrossi@hotmail.com
    Entity: UFFS - Chapecó
*/

Router newRouter(int routerId)
{
    Router router;

    router.id = routerId;

    router._Config = getRouterConfig(routerId);
    router._Network = getLinkConfig();

    router.routerTable = createRouterTable(router);

    return router;
}

List createRouterTable(Router router)
{

    List networkGraph = newGraph();

    Node *currentLink = router._Network.links.first;
    while (currentLink != NULL)
    {
        NetworkNode *data = (NetworkNode *)currentLink->data;
        addGraphEdge(&networkGraph, data->origin,
                     data->destiny, data->cost);

        addGraphEdge(&networkGraph, data->destiny,
            data->origin, data->cost);
   

        currentLink = currentLink->next;
    }

    Node *node = networkGraph.first;
    while (node != NULL)
    {
        GraphNode *graphNode = (GraphNode *)node->data;

        printf("NodeId: %d\n", graphNode->id);
        printf("AdjacencyList: ");
        Node *currentEdge = graphNode->adjacency.first;
        while (currentEdge != NULL)
        {

            GraphEdge *edge = (GraphEdge *)currentEdge->data;

            printf("(%d, %d) ", edge->destiny, edge->cost);

            currentEdge = currentEdge->next;
        }

        printf("\n");
        node = node->next;
    }

    List dijkstra = runDijkstra(&networkGraph, router.id);

    Node *currentNode = dijkstra.first;
    DijkstraResponse *currentData = NULL;
    while (currentNode != NULL)
    {
        currentData = (DijkstraResponse *)currentNode->data;
        printf("Cost: %d, Prev: %d, Dest: %d\n", currentData->cost, currentData->prevNode, currentData->destinyNode);
        
        currentNode = currentNode -> next;
    }
}