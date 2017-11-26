RouterConfig getRouterConfig(int routerId)
{
    RouterConfig config;

    FILE *file = fopen(ROUTER_CONFIG_PATH, "r");
    fseek(file, 0, SEEK_SET);

    int currentId;
    char line[256];
    bool founded = FALSE;

    while (!feof(file))
    {
        fgets(line, sizeof(line), file);

        sscanf(line, "%d %d %s\n", &currentId, &config.port, config.ip);
        if (currentId == routerId)
        {
            founded = TRUE;
            break;
        }
    }

    if (!founded)
    {
        printf(ERROR_ROUTER_NOT_FOUND, routerId);
        exit(EXIT_FAILURE);
    }

    config.id = routerId;

    return config;
}

LinkConfig getLinkConfig(int routerId)
{
    LinkConfig config;
    char line[256];
    FILE *file = fopen(LINK_CONFIG_PATH, "r");
    fseek(file, 0, SEEK_SET);

    config.links = listNew(sizeof(Link));
    config.configs = listNew(sizeof(RouterConfig));

    while (!feof(file))
    {
        Link node;
        int origin, destination;

        fgets(line, sizeof(line), file);
        sscanf(line, "%d %d %d\n", &origin, &destination, &node.cost);

        if (routerId == origin)
            node.destination = destination;
        else if (routerId == destination)
            node.destination = origin;

        if (routerId == origin || routerId == destination)
        {
            listAppend(&config.links, &node);
            
            RouterConfig destinationConfig = getRouterConfig(node.destination);
            listAppend(&config.configs, &destinationConfig);
        }
    }
    return config;
}