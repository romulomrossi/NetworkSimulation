#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "src/defines.h"
#include "src/list.h"
#include "src/config.h"
#include "src/graph.h"
#include "src/router.h"

bool compair(void *a, void *b)
{
    RouterConfig *ra = (RouterConfig *) a;
    RouterConfig *rb = (RouterConfig *) b;
    
    return ra->routerId == rb->routerId;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf(ERROR_NULL_ROUTER_ID);
        exit(EXIT_FAILURE);
    }

    int routerId = atoi(argv[1]);

    Router router = newRouter(routerId);

    RouterConfig testSearch;
    testSearch.routerId = 4;

    RouterConfig *found = (RouterConfig *) listSearchNode(&router._Network.configs, &testSearch, &compair)->data;
    printf("\n%d %d %s \n", found->routerId, found->port, found->ip);
    
}
