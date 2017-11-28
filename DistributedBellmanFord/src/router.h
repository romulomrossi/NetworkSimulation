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

typedef enum { Control, Message } PacketType;

typedef struct _Route
{
    int destinationId;
    RouterConfig nextHop;
    int cost;
} Route;

typedef struct _DistanceVector
{
    int originNode;
    List routes; // List of Routes (nextHop will be not used here)
} DistanceVector;

typedef struct _Packet
{
    PacketType type;
    char content[MESSAGE_SIZE];
    int destinationId;
} Packet;

typedef struct _Router
{
    RouterConfig _Config; 
    List routerTable; //list of Routes
    List distanceVectors; //list of DistanceVectors
    List buffer; //list of Packets
    List neighborsConfigs; //List of RouterConfigs
    pthread_mutex_t bufferLock;
    pthread_mutex_t routerTableLock;
    pthread_mutex_t distanceVectorsLock;
} Router;


Router newRouter(int routerId);
void initializeDistanceVectors(Router *router);
void updateRouterTable(Router *router);
void updateRouterTableLine(Router *router, int destination, int nextHopId, int cost);
RouterConfig getRouteNextHop(Router *router, int destination);
void setRouteAsDead(Router *router, RouterConfig *destinationNode);

void proccessControlPacket(Router *router, Packet *receivedData);

char* serializeDistanceVector(DistanceVector *vector);
DistanceVector *deserializeDistanceVector(char *input);

void createPrintThread(char *str);
void printRouterTable(Router *router);

void *routerHeard(void *data);
void *routerTalk(void *data);

bool addPacketToBuffer(Router *router, Packet *packet);
RouterConfig *getNextHopInfo(Router *router, int id);
int getNextHop(Router *router, Packet *dest);

void shareDistanceVector(Router *router);

#include "router.c"