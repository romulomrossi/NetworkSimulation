#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

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
}
