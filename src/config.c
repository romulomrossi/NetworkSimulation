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

    config.routerId = routerId;

    return config;
}

LinkConfig getLinkConfig()
{
    LinkConfig config;
    char line[256];
    FILE *file = fopen(LINK_CONFIG_PATH, "r");
    fseek(file, 0, SEEK_SET);

    config.links = listNew(sizeof(NetworkNode));
    config.configs = listNew(sizeof(RouterConfig));

    while (!feof(file))
    {
        NetworkNode node;

        fgets(line, sizeof(line), file);
        sscanf(line, "%d %d %d\n", &node.origin, &node.destiny, &node.cost);

        listAppend(&config.links, &node);
    }

    file = fopen(ROUTER_CONFIG_PATH, "r");

    while (!feof(file))
    {
        RouterConfig routerConfig;
        fgets(line, sizeof(line), file);

        sscanf(line, "%d %d %s\n", &routerConfig.routerId, &routerConfig.port, routerConfig.ip);

        listAppend(&config.configs, &routerConfig);
    }
    return config;
}