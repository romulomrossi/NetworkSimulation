/*
    Author: Rômulo Rossi
    Email: romulomrossi@hotmail.com
    Entity: UFFS - Chapecó
*/
typedef Node ListNode;
typedef List *Graph;    //List of GraphNode elements
typedef List *Dijkstra; //List of DijkstraResponse elements

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

typedef struct _DijkstraResponse
{
    int destinyNode;
    int cost;
    struct _DijkstraResponse *prevNode;
    bool done;
    List path; //List of integers
} DijkstraResponse;

List newGraph();
GraphNode newGraphNode(int id);

void addGraphEdge(Graph graph, int idOrigin, int idDestiny, int cost);
GraphNode *getGraphNode(Graph graph, int id);

List initializeDijkstra(Graph graph, int originNode);
List runDijkstra(Graph graph, int originNode);
DijkstraResponse *decideNext(Dijkstra dijkstra);
void dijkstraBacktrack(Dijkstra dijkstraResponse);
void relaxEdges(Dijkstra dijkstra, GraphNode *graphNode, DijkstraResponse *dijkstraResponse);
void dijkstraGetPath(DijkstraResponse *node);

void printGraph(List *graph);

#include "graph.c"