/*
    Author: Rômulo Rossi
    Email: romulomrossi@hotmail.com
    Entity: UFFS - Chapecó
*/
void *lockPrint(void *data)
{
    char *string = (char *)data;

    pthread_mutex_lock(&consoleMutex);

    printf("%s", string);

    pthread_mutex_unlock(&consoleMutex);
}

Router newRouter(int routerId)
{
    Router router;

    router.id = routerId;

    router._Config = getRouterConfig(routerId);
    router._Network = getLinkConfig();

    router.buffer = listNew(sizeof(Packet));
    listSetMaxSize(&router.buffer, BUFFER_SIZE);

    pthread_mutex_init(&router.bufferLock, NULL);

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

int getNextStep(Router *router, Packet *dest)
{
    Node *current = router->routerTable.first;
    Route *currentData = NULL;

    while (current != NULL)
    {
        currentData = (Route *)current->data;

        if (currentData->destiny == dest->destinationId)
            if (currentData->nextNode > -1)
            {
                dest->nextStep = currentData->nextNode;
                return TRUE;
            }

        current = current->next;
    }

    char userResponse[500];
    sprintf(userResponse, ERROR_UNREACHEABLE_ROUTER, dest->destinationId);
    createPrintThread(userResponse);
    return FALSE;
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

        if (receivedData->destinationId == router->id)
        {
            sprintf(userResponse, "\nReceived packet from %s: %d \n Data: %s\n",
                    inet_ntoa(originAddr.sin_addr), ntohs(originAddr.sin_port), receivedData->content);
        }
        else
        {
            sprintf(userResponse, "Received packet from %s:%d, it will be sent to router: %d\n",
                    inet_ntoa(originAddr.sin_addr), ntohs(originAddr.sin_port), receivedData->destinationId);
            if (getNextStep(router, receivedData))
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

void createPrintThread(char *str)
{
    pthread_t threadId;
    pthread_create(&threadId, NULL, lockPrint, str);
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
                    destinationConfig = getDestinationInfo(router, packet);

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
                    printf("Info: sending packet to router %d\n", destinationConfig->routerId);

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
                pthread_mutex_unlock(&router->bufferLock);
            }
            usleep(1);
        }
    }
}

RouterConfig *getDestinationInfo(Router *router, Packet *packet)
{
    Node *currentNodeInfo = router->_Network.configs.first;
    RouterConfig *currentInfo = NULL;
    while (currentNodeInfo != NULL)
    {
        currentInfo = (RouterConfig *)currentNodeInfo->data;

        if (currentInfo->routerId == packet->nextStep)
            return currentInfo;

        currentNodeInfo = currentNodeInfo->next;
    }
    return NULL;
}