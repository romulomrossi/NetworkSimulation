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
    printf("\n\n");
    printf(LEFT_SEPARATOR "ROUTER TABLE" RIGHT_SEPARATOR);

    Node *current = router->routerTable.first;
    Route *route = NULL;
    while (current != NULL)
    {
        route = (Route *)current->data;

        printf("Destiny: %d, NextStep: %d \n", route->destinationId, route->nextHop.id);

        current = current->next;
    }
}

void updateRouterTable(Router *router)
{
    DistanceVector *vector = (DistanceVector *)router->distanceVectors.first->data;
    Node *iterator = vector->routes.first;

    pthread_mutex_lock(&router->routerTableLock);
    while (iterator != NULL)
    {
        Route *route = (Route *)iterator->data;
        updateRouterTableLine(router, route->destinationId, route->nextHop.id);

        iterator = iterator->next;
    }
    pthread_mutex_unlock(&router->routerTableLock);
}

void updateRouterTableLine(Router *router, int destination, int nextHopId)
{
    Route newRoute;
    newRoute.destinationId = destination;
    newRoute.nextHop = getRouteNextHop(router, nextHopId);

    Node *existentRoute = listSearchNode(&router->routerTable, &newRoute, compareRoutes);

    if (existentRoute != NULL)
        existentRoute->data = &newRoute;
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

DistanceVector newDistanceVector(int origin)
{
    DistanceVector vector;

    vector.originNode = origin;
    vector.routes = listNew(sizeof(Route));

    return vector;
}

void initializeDistanceVectors(Router *router)
{
    LinkConfig neighborhood = getLinkConfig(router->_Config.id);
    router->distanceVectors = listNew(sizeof(DistanceVector));
    router->neighborsConfigs = neighborhood.configs;
    DistanceVector distanceVector = newDistanceVector(router->_Config.id);

    Node *iterator = neighborhood.links.first;
    while (iterator != NULL)
    {
        Link *link = (Link *)iterator->data;

        Route route;
        route.destinationId = link->destination;
        route.cost = link->cost;
        route.nextHop.id = link->destination;
        listAppend(&distanceVector.routes, &route);

        iterator = iterator->next;
    }

    listAppend(&router->distanceVectors, &distanceVector);
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
            response = listPreppend(&router->buffer, packet);
            pthread_mutex_unlock(&router->bufferLock);
            break;
        }
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
        printf(ERROR_SOCKET_CREATE " line 170");
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
            sprintf(userResponse, "\nReceived packet from %s: %d \n Data: %s\n",
                    inet_ntoa(originAddr.sin_addr), ntohs(originAddr.sin_port), receivedData->content);
        }
        else
        {
            sprintf(userResponse, "Received packet from %s:%d, it will be sent to router: %d\n",
                    inet_ntoa(originAddr.sin_addr), ntohs(originAddr.sin_port), receivedData->destinationId);
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

void *routerTalk(void *data)
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
                RouterConfig *destinationConfig = NULL;
                acquired = FALSE;
                if (router->buffer.count > 0)
                {
                    packetReference = listPop(&router->buffer, router->buffer.last);
                    packet = (Packet *)packetReference->data;
                    pthread_mutex_unlock(&router->bufferLock);                    
                    destinationConfig = getDestinationInfo(router, packet->destinationId);

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
                    socketAddress.sin_port = htons(destinationConfig->port);

                    // set ip to destination ip
                    if (inet_aton(destinationConfig->ip, &socketAddress.sin_addr) == 0)
                    {
                        fprintf(stderr, "Sending: inet_aton() failed\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("Info: sending packet to router %d\n", destinationConfig->id);

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
                        createPrintThread(ERROR_MESSAGE_ABORT);

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

RouterConfig *getDestinationInfo(Router *router, int id)
{
    pthread_mutex_lock(&router->routerTableLock);

    Node *iterator = router->routerTable.first;
    while (iterator != NULL)
    {
        Route *route = (Route *)iterator->data;
        if (route->destinationId == id)
            return &route->nextHop;

        iterator = iterator->next;
    }

    pthread_mutex_unlock(&router->routerTableLock);
}
