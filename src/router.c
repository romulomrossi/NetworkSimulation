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

    printf(LEFT_SEPARATOR "NETWORK GRAPH" RIGHT_SEPARATOR);
    printGraph(&networkGraph);

    List dijkstra = runDijkstra(&networkGraph, router.id);
    dijkstraBacktrack(&dijkstra);

    Node *currentNode = dijkstra.first;
    DijkstraResponse *currentData = NULL;
    while (currentNode != NULL)
    {
        currentData = (DijkstraResponse *)currentNode->data;
        DijkstraResponse *prevNode = ((DijkstraResponse *)currentData->prevNode);
        printf("Cost: %d, Prev: %d, Dest: %d Path: ", currentData->cost,
               prevNode != NULL ? prevNode->destinyNode : -1, currentData->destinyNode);

        Node *currentPrevNode = currentData->path.first;
        while (currentPrevNode != NULL)
        {
            printf("%d ", *(int *)currentPrevNode->data);
            currentPrevNode = currentPrevNode->next;
        }

        printf("\n");
        currentNode = currentNode->next;
    }
}