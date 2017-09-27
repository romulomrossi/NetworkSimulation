/*
    Author: Rômulo Rossi
    Email: romulomrossi@hotmail.com
    Entity: UFFS - Chapecó
*/

/*
    Config will store information about network and routers
        -The ip address and port tath each router will being heard;
        -The network existent links;
*/

typedef struct _RouterConfig
{
    int routerId;
    int port;
    char ip[255];
} RouterConfig;

typedef struct _Link
{
    int origin;
    int destiny;
    int cost;
} NetworkNode;

typedef struct _LinkConfig
{
    List links;
    List configs;
} LinkConfig;

RouterConfig getRouterConfig(int routerId);
LinkConfig getLinkConfig();

#include "config.c"