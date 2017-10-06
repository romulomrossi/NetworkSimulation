/*
    Author: Rômulo Rossi
    Email: romulomrossi@hotmail.com
    Entity: UFFS - Chapecó
*/

/*
    The router will be:
        -Recive a message from other;
            -Handle this message;
                -It's for me? So, i print it;
                -It's for a neighbor? Send her!
        -Send message for a network router member.
*/

typedef struct _Route
{
    int destiny;
    int nextNode;
    int cost;
} Route;

typedef struct _Router
{
    int id;
    List routerTable;
    List buffer;
    pthread_mutex_t bufferLock;
    LinkConfig _Network;
    RouterConfig _Config;
} Router;

typedef struct _Packet
{
    long id;
    char content[MESSAGE_SIZE];
    int destinationId;
    int nextStep;
} Packet;

Router newRouter(int routerId);
List createRouterTable(Router *router);
void setUnreachableNodes(List *routerTable);

void createPrintThread(char *str);

void printRouterTable(Router *router);

void *routerHeard(void *data);
void *routerTalk(void *data);

bool addPacketToBuffer(Router *router, Packet *packet);
RouterConfig *getDestinationInfo(Router *router, Packet *packet);
int getNextStep(Router *router, Packet *dest);

#include "router.c"