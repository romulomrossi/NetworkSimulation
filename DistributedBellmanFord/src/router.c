/*
    Author: Rômulo Rossi
    Email: romulomrossi@hotmail.com
    Entity: UFFS - Chapecó
*/
Router newRouter(int routerId)
{
    Router router;
    router._Config.id = routerId;

    router._Config = getRouterConfig(routerId);
    router.buffer = listNew(sizeof(Packet));
    listSetMaxSize(&router.buffer, BUFFER_SIZE);

    pthread_mutex_init(&router.bufferLock, NULL);
    pthread_mutex_init(&router.routerTableLock, NULL);
    pthread_mutex_init(&router.distanceVectorsLock, NULL);

    initializeDistanceVectors(&router);
    router.routerTable = listNew(sizeof(Route));

    updateRouterTable(&router);
    printRouterTable(&router);
    shareDistanceVector(&router);
    return router;
}

bool compareDistanceVectors(void *a, void *b)
{
    DistanceVector *vectorA = (DistanceVector *)a;
    DistanceVector *vectorB = (DistanceVector *)b;

    return vectorA->originNode == vectorB->originNode;
}

bool compareRouterConfigs(void *a, void *b)
{
    RouterConfig *routerA = (RouterConfig *)a;
    RouterConfig *routerB = (RouterConfig *)b;

    return routerA->id == routerB->id;
}

bool compareRoutes(void *a, void *b)
{
    Route *routeA = (Route *)a;
    Route *routeB = (Route *)b;

    return routeA->destinationId == routeB->destinationId;
}

void printRouterTable(Router *router)
{
    pthread_mutex_lock(&consoleMutex);
    printf("\n\n");
    printf(LEFT_SEPARATOR "ROUTER TABLE" RIGHT_SEPARATOR);

    Node *current = router->routerTable.first;
    Route *route = NULL;
    while (current != NULL)
    {
        route = (Route *)current->data;

        printf("Destiny: %d, NextStep: %d, Cost: %d\n", route->destinationId,
               route->nextHop.id, route->cost);

        current = current->next;
    }
    pthread_mutex_unlock(&consoleMutex);
}

void updateRouterTable(Router *router)
{
    pthread_mutex_lock(&router->distanceVectorsLock);
    DistanceVector *vector = (DistanceVector *)router->distanceVectors.first->data;
    pthread_mutex_unlock(&router->distanceVectorsLock);

    Node *iterator = vector->routes.first;
    pthread_mutex_lock(&router->routerTableLock);
    while (iterator != NULL)
    {
        Route *route = (Route *)iterator->data;
        updateRouterTableLine(router, route->destinationId, route->nextHop.id, route->cost);

        iterator = iterator->next;
    }
    pthread_mutex_unlock(&router->routerTableLock);
    printRouterTable(router);
}

void *updateDistanceVector(void *data)
{
    Router *router = (Router *)data;
    int sleepTime = 2;
    while (TRUE)
    {
        pthread_mutex_lock(&router->distanceVectorsLock);
        DistanceVector *ownVector = (DistanceVector *)router->distanceVectors.first->data;

        Node *iterator = router->distanceVectors.first->next;
        while (iterator != NULL)
        {
            DistanceVector *neighborVector = (DistanceVector *)iterator->data;
            if (neighborVector == NULL)
                continue;
            RouterConfig comp;
            comp.id = neighborVector->originNode;
            Node *neighborConfigRef = listSearchNode(&router->neighborsConfigs,
                                                     &comp, compareRouterConfigs);
            RouterConfig *neighborConfig = (RouterConfig *)neighborConfigRef->data;
            if (neighborConfig == NULL)
                continue;

            Node *routeIterator = neighborVector->routes.first;
            while (routeIterator != NULL)
            {
                Route *neighborRoute = (Route *)routeIterator->data;
                Node *ownRouteRef = listSearchNode(&ownVector->routes, neighborRoute, compareRoutes);

                neighborRoute->nextHop.id = neighborConfig->id;
                neighborRoute->cost += neighborConfig->linkCost;

                if(neighborRoute->destinationId != router->_Config.id)
                {
                    if (ownRouteRef == NULL)
                        listAppend(&ownVector->routes, neighborRoute);
                    else 
                    {
                        Route *ownRoute = (Route *)ownRouteRef->data;
                        if (neighborRoute->cost < ownRoute->cost)
                        {
                            ownRoute->cost = neighborRoute->cost;
                            ownRoute->nextHop.id = neighborConfig->id;
                        }
                    }
                }


                routeIterator = routeIterator->next;
            }

            iterator = iterator->next;
        }
        pthread_mutex_unlock(&router->distanceVectorsLock);
        updateRouterTable(router);
        shareDistanceVector(router);
        sleep(sleepTime);
    }
}

void updateRouterTableLine(Router *router, int destination, int nextHopId, int cost)
{
    Route newRoute;
    newRoute.destinationId = destination;
    newRoute.nextHop = getRouteNextHop(router, nextHopId);
    newRoute.cost = cost;
    Node *existentRoute = listSearchNode(&router->routerTable, &newRoute, compareRoutes);

    if (existentRoute != NULL)
    {
        Route *route = (Route *)existentRoute->data;
        route->destinationId = newRoute.destinationId;
        route->nextHop = newRoute.nextHop;
        route->cost = newRoute.cost;
    }
    else
        listAppend(&router->routerTable, &newRoute);
}

RouterConfig getRouteNextHop(Router *router, int destination)
{
    RouterConfig config;
    config.id = destination;
    Node *listNode = listSearchNode(&router->neighborsConfigs, &config, compareRouterConfigs);

    return (*((RouterConfig *)listNode->data));
}

DistanceVector *newDistanceVector(int origin)
{
    DistanceVector *vector = (DistanceVector *)malloc(sizeof(DistanceVector));

    vector->originNode = origin;
    vector->routes = listNew(sizeof(Route));

    return vector;
}

void initializeDistanceVectors(Router *router)
{
    LinkConfig neighborhood = getLinkConfig(router->_Config.id);
    router->distanceVectors = listNew(sizeof(DistanceVector));
    router->neighborsConfigs = neighborhood.configs;
    DistanceVector *distanceVector = newDistanceVector(router->_Config.id);

    Node *iterator = neighborhood.links.first;
    while (iterator != NULL)
    {
        Link *link = (Link *)iterator->data;

        Route *route = malloc(sizeof(Route));
        route->destinationId = link->destination;
        route->cost = link->cost;
        route->nextHop.id = link->destination;
        listAppend(&distanceVector->routes, route);

        iterator = iterator->next;
    }

    listAppend(&router->distanceVectors, distanceVector);
}

void shareDistanceVector(Router *router)
{
    DistanceVector *vector = (DistanceVector *)router->distanceVectors.first->data;
    pthread_mutex_lock(&router->distanceVectorsLock);
    char *serializedVector = serializeDistanceVector(vector);
    pthread_mutex_unlock(&router->distanceVectorsLock);

    Node *nIterator = router->neighborsConfigs.first;
    while (nIterator != NULL)
    {
        RouterConfig *config = (RouterConfig *)nIterator->data;

        Packet *p = malloc(sizeof(Packet) + MESSAGE_SIZE);
        p->destinationId = config->id;
        sprintf(p->content, "%s", serializedVector);

        addPacketToBuffer(router, p);
        nIterator = nIterator->next;
    }
}

char *serializeDistanceVector(DistanceVector *vector)
{
    /*
        The distance vector will be serialized in the next format:
            [OiginId](nodeA,distance)|(nodeB,distance)
    */
    char *content = (char *)malloc(MESSAGE_SIZE);
    sprintf(content, "[%d]", vector->originNode);

    Node *iterator = vector->routes.first;
    while (iterator != NULL)
    {
        Route *distance = (Route *)iterator->data;
        char strDistance[10];
        sprintf(strDistance, "(%d,%d)|", distance->destinationId, distance->cost);

        strcat(content, strDistance);
        iterator = iterator->next;
    }
    content[strlen(content) - 1] = '\0';
    return content;
}

DistanceVector *deserializeDistanceVector(char *input)
{
    char *p = strtok(input, "]");

    int originNode;
    sscanf(p, "[%d", &originNode);
    DistanceVector *vector = newDistanceVector(originNode);

    p = strtok(NULL, "|");
    while (p != NULL)
    {
        Route route;
        int cost, dest;
        sscanf(p, "(%d,%d)", &dest, &cost);

        route.destinationId = dest;
        route.cost = cost;

        listAppend(&vector->routes, &route);
        p = strtok(NULL, "|");
    }

    return vector;
}

void *lockPrint(void *data)
{
    char *string = (char *)data;

    pthread_mutex_lock(&consoleMutex);

    printf("%s", string);

    pthread_mutex_unlock(&consoleMutex);
}

void createPrintThread(char *str)
{
    pthread_t threadId;
    pthread_create(&threadId, NULL, lockPrint, str);
}

bool addPacketToBuffer(Router *router, Packet *packet)
{
    bool response = FALSE;
    while (TRUE)
    {
        if (!pthread_mutex_trylock(&router->bufferLock))
        {
            response = listAppend(&router->buffer, packet);
            pthread_mutex_unlock(&router->bufferLock);
            break;
        }
        usleep(10);
    }
    return response;
}

void *routerHeard(void *data)
{
    Router *router = (Router *)data;
    struct sockaddr_in localAddr, originAddr;
    int socketId, receivedLen, ack = 1;
    socklen_t slen = sizeof(originAddr);

    memset((char *)&localAddr, 0, sizeof(localAddr));

    if ((socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf(ERROR_SOCKET_CREATE " line in routerHeard()");
        exit(EXIT_FAILURE);
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(router->_Config.port);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socketId, (struct sockaddr *)&localAddr, sizeof(localAddr)) == -1)
    {
        printf(ERROR_SOCKET_CREATE " line 180");
        exit(EXIT_FAILURE);
    }

    while (TRUE)
    {

        Packet *receivedData = (Packet *)malloc(sizeof(Packet) + MESSAGE_SIZE);
        fflush(stdout);
        char userResponse[MESSAGE_SIZE * 2];

        if ((receivedLen = recvfrom(socketId, receivedData, sizeof(Packet), 0, (void *)&originAddr, &slen)) == -1)
            exit(EXIT_FAILURE);

        if (receivedData->destinationId == router->_Config.id)
        {
            switch (receivedData->type)
            {
            case Message:
                sprintf(userResponse, "\nReceived packet from %s: %d \n Data: %s\n",
                        inet_ntoa(originAddr.sin_addr), ntohs(originAddr.sin_port), receivedData->content);
                break;
            case Control:
                sprintf(userResponse, "\nReceived Control packet from %s: %d \n Data: %s\n",
                        inet_ntoa(originAddr.sin_addr), ntohs(originAddr.sin_port), receivedData->content);
                proccessControlPacket(router, receivedData);
                break;
            }
        }
        else
        {
            //sprintf(userResponse, "Received packet from %s:%d, it will be sent to router: %d\n",
            //      inet_ntoa(originAddr.sin_addr), ntohs(originAddr.sin_port), receivedData->destinationId);
            if (!addPacketToBuffer(router, receivedData))
            {
                printf(ERROR_FULL_BUFFER);
                ack = 0;
            }
        }

        createPrintThread(userResponse);

        if (sendto(socketId, &ack, sizeof(ack), 0, (void *)&originAddr, slen) == -1)
            exit(EXIT_FAILURE);
    }

    free(data);
    close(socketId);
    return 0;
}

void proccessControlPacket(Router *router, Packet *receivedData)
{
    pthread_mutex_lock(&router->distanceVectorsLock);
    DistanceVector *newDistanceVector = deserializeDistanceVector(receivedData->content);
    Node *oldVectorRef = listSearchNode(&router->distanceVectors,
                                        newDistanceVector, compareDistanceVectors);
    if (oldVectorRef != NULL)
        oldVectorRef->data = newDistanceVector;
    else
        listAppend(&router->distanceVectors, newDistanceVector);

    pthread_mutex_unlock(&router->distanceVectorsLock);
}

void *routerTalk(void *data)
{
    {
        while (TRUE)
        {
            bool acquired = TRUE;
            while (acquired)
            {
                Router *router = (Router *)data;
                if (!pthread_mutex_trylock(&router->bufferLock))
                {
                    Packet *packet;
                    Node *packetReference;
                    RouterConfig *nextHopConfig = NULL;
                    acquired = FALSE;
                    if (router->buffer.count > 0)
                    {
                        packetReference = listUnqueue(&router->buffer);
                        pthread_mutex_unlock(&router->bufferLock);
                        packet = (Packet *)packetReference->data;
                        nextHopConfig = getNextHopInfo(router, packet->destinationId);

                        if (nextHopConfig == NULL)
                        {
                            acquired = FALSE;
                            continue;
                        }

                        struct sockaddr_in socketAddress;
                        int socketId;
                        socklen_t socketLen = sizeof(socketAddress);

                        if ((socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
                        {
                            printf(ERROR_SOCKET_CREATE "line 257");
                            exit(EXIT_FAILURE);
                        }

                        memset((char *)&socketAddress, 0, sizeof(socketAddress));
                        socketAddress.sin_family = AF_INET;
                        socketAddress.sin_port = htons(nextHopConfig->port);

                        // set ip to destination ip
                        if (inet_aton(nextHopConfig->ip, &socketAddress.sin_addr) == 0)
                        {
                            fprintf(stderr, "Sending: inet_aton() failed\n");
                            exit(EXIT_FAILURE);
                        }
                        printf("Info: sending packet to router %d\n", nextHopConfig->id);

                        int tries = TRIES;
                        int ack = 0;
                        while (tries--)
                        {
                            if (sendto(socketId, packet, sizeof(Packet) + (MESSAGE_SIZE), 0,
                                       (struct sockaddr *)&socketAddress, socketLen) == -1)
                            {
                                printf(ERROR_SOCKET_CREATE "line 281");
                                exit(EXIT_FAILURE);
                            }

                            struct timeval read_timeout;
                            read_timeout.tv_sec = TIMEOUT;
                            read_timeout.tv_usec = 10;

                            setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

                            if (recvfrom(socketId, &ack, sizeof(ack), 0, (struct sockaddr *)&socketAddress, &socketLen) == -1)
                                createPrintThread(ERROR_MESSAGE_RETRY);

                            if (ack)
                            {
                                createPrintThread(SUCCESS_MESSAGE_SENT);
                                break;
                            }
                        }
                        if (!ack)
                        {
                            createPrintThread(ERROR_MESSAGE_ABORT);
                            setRouteAsDead(router, nextHopConfig);
                        }

                        free(packet);
                        free(packetReference);
                    }
                    else
                        pthread_mutex_unlock(&router->bufferLock);
                }
                usleep(1);
            }
        }
    }
}

void setRouteAsDead(Router *router, RouterConfig *destinationNode)
{
    pthread_mutex_lock(&router->distanceVectorsLock);
    Node *vectorRef = router->distanceVectors.first;
    if (vectorRef != NULL)
    {
        DistanceVector *dv = (DistanceVector *)vectorRef->data;

        Route comp;
        comp.destinationId = destinationNode->id;
        Node *routeRef = listSearchNode(&dv->routes, &comp, compareRoutes);
        if (routeRef != NULL)
        {
            Route *route = (Route *)routeRef->data;
            route->cost = INF;
        }
    }
    DistanceVector c;
    c.originNode = destinationNode->id;
    vectorRef = listSearchNode(&router->distanceVectors, &c, compareDistanceVectors);
    if (vectorRef != NULL)
        listRemove(&router->distanceVectors, vectorRef);
    pthread_mutex_unlock(&router->distanceVectorsLock);
    updateRouterTable(router);
}

RouterConfig *getNextHopInfo(Router *router, int id)
{
    pthread_mutex_lock(&router->routerTableLock);

    Node *iterator = router->routerTable.first;
    while (iterator != NULL)
    {
        Route *route = (Route *)iterator->data;
        if (route->destinationId == id)
        {
            pthread_mutex_unlock(&router->routerTableLock);

            if (route->cost == INF)
                return NULL;

            return &route->nextHop;
        }

        iterator = iterator->next;
    }
    pthread_mutex_unlock(&router->routerTableLock);
    return NULL;
}