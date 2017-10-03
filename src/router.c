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

    router.buffer = listNew(sizeof(Packet));
    listSetMaxSize(&router.buffer, BUFFER_SIZE);

    router.routerTable = listNew(sizeof(Route));
    createRouterTable(&router);
    setUnreachableNodes(&router.routerTable);

    printRouterTable(&router);

    return router;
}

void setUnreachableNodes(List *routerTable)
{
    Node *current = routerTable->first;
    Route *currentData = NULL;
    while (current != NULL)
    {
        currentData = (Route *)current->data;

        if (currentData->cost == INF)
            currentData->nextNode = -1;

        current = current->next;
    }
}

List createRouterTable(Router *router)
{
    List networkGraph = newGraph();

    Node *currentLink = router->_Network.links.first;
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

    List dijkstra = runDijkstra(&networkGraph, router->id);

    Node *currentNode = dijkstra.first;
    DijkstraResponse *currentData = NULL;
    while (currentNode != NULL)
    {
        currentData = (DijkstraResponse *)currentNode->data;

        DijkstraResponse *nextStep = currentData;
        while (TRUE)
        {
            if (nextStep->prevNode == NULL)
                break;
            if (nextStep->prevNode->prevNode == NULL)
                break;
            nextStep = nextStep->prevNode;
        }

        Route route;

        route.nextNode = nextStep->destinyNode;
        route.cost = currentData->cost;
        route.destiny = currentData->destinyNode;

        listAppend(&router->routerTable, &route);
        currentNode = currentNode->next;
    }
}

void printRouterTable(Router *router)
{
    printf("\n\n");
    printf(LEFT_SEPARATOR "ROUTER TABLE" RIGHT_SEPARATOR);

    Node *current = router->routerTable.first;
    Route *route = NULL;
    while (current != NULL)
    {
        route = (Route *)current->data;

        printf("Destiny: %d, Cost: %d, NextStep: %d \n", route->destiny, route->cost, route->nextNode);

        current = current->next;
    }
}

void *routerHeard(void *data)
{
    Router *router = (Router *) data;

    while(TRUE){
        sleep(2);
        printf("Router %d hearding at %s:%d \n", router->id, router->_Config.ip, router->_Config.port);
    }
}

void *routerTalk(void *data)
{
    Router *router = (Router *) data;

    char message[MESSAGE_SIZE];
    while (TRUE)
    {
        scanf("%s", message);

        printf("%s", message);
    }
}