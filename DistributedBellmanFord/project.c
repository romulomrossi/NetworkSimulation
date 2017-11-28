#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t consoleMutex;

#include "src/defines.h"
#include "src/list.h"
#include "src/config.h"
#include "src/router.h"

void printHelp()
{
    printf("\n\n\n");
    printf(LEFT_SEPARATOR "How to use" RIGHT_SEPARATOR "\n");

    printf("Commands:\n");
    printf("\"message\" - Send message for router. \n");
    printf("\"help\" - Show this informations.\n");
    printf("\"rtable\" - Show router table.\n ");

    printf("\n\n");

    printf(LEFT_SEPARATOR "Program running!" RIGHT_SEPARATOR "\n");
}

int main(int argc, char *argv[])
{
    pthread_mutex_init(&consoleMutex, NULL);
    sleep(1);
    if (argc != 2)
    {
        printf(ERROR_NULL_ROUTER_ID);
        exit(EXIT_FAILURE);
    }

    int routerId = atoi(argv[1]);
    pthread_t heardThreadId = 0;
    pthread_t talkThreadId = 0;
    pthread_t distanceVectorThreadId = 0;

    Router router = newRouter(routerId);

    pthread_create(&heardThreadId, NULL, routerHeard, &router);
    pthread_create(&talkThreadId, NULL, routerTalk, &router);
    pthread_create(&distanceVectorThreadId, NULL, updateDistanceVector, &router);

    printHelp();

    while (TRUE)
    {
        char action[50];
        scanf("%s", action);

        if(!strcmp(action, "help"))
            printHelp();

        if(!strcmp(action, "rtable"))
            printRouterTable(&router);

        if (strcmp(action, "message"))
            continue;

        printf("\n\n");
        
        pthread_mutex_lock(&consoleMutex);
        
        Packet message;
        message.type = 0;
        printf("Tap the router that you need to talk: ");
        scanf("%d", &message.destinationId);
        getchar();
        printf("Tap your message: \n ");

        fgets(message.content, MESSAGE_SIZE, stdin);

        int messageLenght = strlen(message.content);
        message.content[--messageLenght] = '\0';

        if (!addPacketToBuffer(&router, &message))
            printf(ERROR_FULL_BUFFER);

        pthread_mutex_unlock(&consoleMutex);
    }
}
