/*
    Author: Rômulo Rossi
    Email: romulomrossi@hotmail.com
    Entity: UFFS - Chapecó
*/
typedef struct _adj
{
    int destiny;
    int cost;
} GraphEdge;

typedef struct _graphNode
{
    int id;
    List adjacency; //List of graphEdges
} GraphNode;

List newGraph();
GraphNode newGraphNode(int id);

void addGraphEdge(List *graph, int idOrigin, int idDestiny, int cost);
GraphNode *getGraphNode(List *graph, int id);

#include "graph.c"