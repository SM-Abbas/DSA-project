#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FileOperations.h"
#include "Route.h"
#include "GraphFunctions.h"

int main(int argc, char* argv[]) {
    char citiesFilename[256] = {0};
    char routesFilename[256] = {0};
    char outputFilename[256] = {0};
    char origin[256] = {0};
    char destination[256] = {0};
    char preference[256] = {0};
    int biPreference = 0;

    if (argc > 1) {
        strcpy(citiesFilename, argv[1]);
    } else {
        printf("Enter filename containing cities: ");
        scanf("%255s", citiesFilename);
    }

    if (argc > 2) {
        strcpy(routesFilename, argv[2]);
    } else {
        printf("Enter filename containing routes: ");
        scanf("%255s", routesFilename);
    }

    if (argc > 3) {
        strcpy(outputFilename, argv[3]);
    } else {
        printf("Enter filename for output (.html): ");
        scanf("%255s", outputFilename);
    }

    if (argc > 4) {
        strcpy(origin, argv[4]);
    } else {
        printf("Enter origin city: ");
        scanf("%255s", origin);
    }

    if (argc > 5) {
        strcpy(destination, argv[5]);
    } else {
        printf("Enter destination city: ");
        scanf("%255s", destination);
    }

    if (argc > 6) {
        strcpy(preference, argv[6]);
    } else {
        printf("Enter preference (cost or time): ");
        scanf("%255s", preference);
    }

    if (strcmp(preference, "cost") == 0) {
        biPreference = 1;
    } else {
        biPreference = 0;
    }

    Graph* graph = createGraph(citiesFilename, routesFilename);
    if (graph == NULL) {
        printf("Failed to create graph\n");
        return 1;
    }

    dijkstras(graph, origin, biPreference);

    Stack* cityStack = cityStacker(graph, destination);
    Stack* routeStack = routeStacker(graph, destination, biPreference);

    generateOutput(outputFilename, cityStack, routeStack, biPreference);

    // Free memory
    freeGraph(graph);
    freeStack(cityStack);
    freeStack(routeStack);

    return 0;
} 