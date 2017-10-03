#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#include "src/defines.h"
#include "src/list.h"
#include "src/config.h"
#include "src/graph.h"
#include "src/router.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf(ERROR_NULL_ROUTER_ID);
        exit(EXIT_FAILURE);
    }

    int routerId = atoi(argv[1]);
    pthread_t heardThreadId = 0;
    pthread_t talkThreadId = 0;

    Router router = newRouter(routerId);

    pthread_create(&heardThreadId, NULL, routerHeard, &router);
    pthread_create(&talkThreadId, NULL, routerTalk, &router);

    while (TRUE)
    {
        Packet message;
        scanf("Tap the router that you need to talk: %d", &message.destinationRouter.routerId);
        scanf("Tap your message: \n %s", message.content);

        addMessageToBuffer(&router, message);
    }
}
