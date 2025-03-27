#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Location.h"
#include "Route.h"

// Forward declarations
struct Graph;
typedef struct Graph Graph;
struct Stack;
typedef struct Stack Stack;

// Function prototypes
int loadRoutesAndCities(Graph* graph, const char* citiesFilename, const char* routesFilename);
int loadCities(Graph* graph, const char* filename);
int loadRoutes(Graph* graph, const char* filename);
void generateOutput(const char* filename, Stack* cities, Stack* routes, int costOrTime);

// Implementation
int loadRoutesAndCities(Graph* graph, const char* citiesFilename, const char* routesFilename) {
    if (graph == NULL || citiesFilename == NULL || routesFilename == NULL) {
        return 0;
    }
    
    // First load routes
    if (!loadRoutes(graph, routesFilename)) {
        return 0;
    }
    
    // Then load cities and connect them to routes
    if (!loadCities(graph, citiesFilename)) {
        return 0;
    }
    
    return 1;
}

int loadCities(Graph* graph, const char* filename) {
    if (graph == NULL || filename == NULL) {
        return 0;
    }
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening cities file: %s\n", filename);
        return 0;
    }
    
    char line[1024];
    char country[256];
    char city[256];
    char latitude[32];
    char longitude[32];
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse CSV line
        char* token = strtok(line, ",");
        if (token == NULL) continue;
        strncpy(country, token, sizeof(country) - 1);
        country[sizeof(country) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(city, token, sizeof(city) - 1);
        city[sizeof(city) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(latitude, token, sizeof(latitude) - 1);
        latitude[sizeof(latitude) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(longitude, token, sizeof(longitude) - 1);
        longitude[sizeof(longitude) - 1] = '\0';
        
        // Create location
        Location* node = createLocationWithCoords(country, city, atof(latitude), atof(longitude));
        if (node == NULL) {
            continue;
        }
        
        // Connect routes to this city
        for (int i = 0; i < graph->routeCount; i++) {
            Route* route = graph->routes[i];
            
            if (strcmp(route->originS, node->capital) == 0) {
                route->origin = node;
                addRouteToLocation(node, route);
            }
            else if (strcmp(route->destinationS, node->capital) == 0) {
                route->destination = node;
            }
        }
        
        // Add city to graph
        if (graph->cityCount >= graph->cityCapacity) {
            int newCapacity = graph->cityCapacity == 0 ? 10 : graph->cityCapacity * 2;
            Location** newCities = (Location**)realloc(graph->cities, newCapacity * sizeof(Location*));
            if (newCities == NULL) {
                freeLocation(node);
                continue;
            }
            graph->cities = newCities;
            graph->cityCapacity = newCapacity;
        }
        
        graph->cities[graph->cityCount++] = node;
    }
    
    fclose(file);
    printf("Cities Parsed from: %s\n", filename);
    
    return 1;
}

int loadRoutes(Graph* graph, const char* filename) {
    if (graph == NULL || filename == NULL) {
        return 0;
    }
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening routes file: %s\n", filename);
        return 0;
    }
    
    char line[1024];
    char origin[256];
    char destination[256];
    char transport[256];
    char time[32];
    char cost[32];
    char note[256];
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse CSV line
        char* token = strtok(line, ",");
        if (token == NULL) continue;
        strncpy(origin, token, sizeof(origin) - 1);
        origin[sizeof(origin) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(destination, token, sizeof(destination) - 1);
        destination[sizeof(destination) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(transport, token, sizeof(transport) - 1);
        transport[sizeof(transport) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(time, token, sizeof(time) - 1);
        time[sizeof(time) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strncpy(cost, token, sizeof(cost) - 1);
        cost[sizeof(cost) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token != NULL) {
            strncpy(note, token, sizeof(note) - 1);
            note[sizeof(note) - 1] = '\0';
        } else {
            note[0] = '\0';
        }
        
        // Create route
        Route* route = createRoute();
        if (route == NULL) {
            continue;
        }
        
        strncpy(route->originS, origin, sizeof(route->originS) - 1);
        route->originS[sizeof(route->originS) - 1] = '\0';
        
        strncpy(route->destinationS, destination, sizeof(route->destinationS) - 1);
        route->destinationS[sizeof(route->destinationS) - 1] = '\0';
        
        strncpy(route->transport, transport, sizeof(route->transport) - 1);
        route->transport[sizeof(route->transport) - 1] = '\0';
        
        route->time = atof(time);
        route->cost = atof(cost);
        
        strncpy(route->note, note, sizeof(route->note) - 1);
        route->note[sizeof(route->note) - 1] = '\0';
        
        // Add route to graph
        if (graph->routeCount >= graph->routeCapacity) {
            int newCapacity = graph->routeCapacity == 0 ? 10 : graph->routeCapacity * 2;
            Route** newRoutes = (Route**)realloc(graph->routes, newCapacity * sizeof(Route*));
            if (newRoutes == NULL) {
                freeRoute(route);
                continue;
            }
            graph->routes = newRoutes;
            graph->routeCapacity = newCapacity;
        }
        
        graph->routes[graph->routeCount++] = route;
    }
    
    fclose(file);
    printf("Routes Parsed from: %s\n", filename);
    
    return 1;
}

void generateOutput(const char* filename, Stack* cities, Stack* routes, int costOrTime) {
    if (filename == NULL || cities == NULL || routes == NULL) {
        return;
    }
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening output file: %s\n", filename);
        return;
    }
    
    fprintf(file, "<!DOCTYPE html>\n");
    fprintf(file, "<html>\n");
    fprintf(file, "<head>\n");
    fprintf(file, "    <title>Travel Route</title>\n");
    fprintf(file, "    <style>\n");
    fprintf(file, "        body { font-family: Arial, sans-serif; margin: 20px; }\n");
    fprintf(file, "        h1 { color: #333; }\n");
    fprintf(file, "        .route { margin-bottom: 20px; }\n");
    fprintf(file, "        .route-item { margin: 10px 0; padding: 10px; border: 1px solid #ddd; border-radius: 5px; }\n");
    fprintf(file, "        .summary { font-weight: bold; margin-top: 20px; }\n");
    fprintf(file, "    </style>\n");
    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");
    fprintf(file, "    <h1>Travel Route</h1>\n");
    fprintf(file, "    <div class=\"route\">\n");
    
    // Create temporary stacks to preserve the original stacks
    Stack* tempCities = createStack();
    Stack* tempRoutes = createStack();
    
    // Copy cities to temp stack (in reverse order)
    while (!isEmpty(cities)) {
        Location* city = (Location*)pop(cities);
        push(tempCities, city);
    }
    
    // Copy routes to temp stack (in reverse order)
    while (!isEmpty(routes)) {
        Route* route = (Route*)pop(routes);
        push(tempRoutes, route);
    }
    
    // Restore original stacks and print route
    float totalCost = 0;
    float totalTime = 0;
    
    // Print first city
    if (!isEmpty(tempCities)) {
        Location* city = (Location*)pop(tempCities);
        fprintf(file, "        <div class=\"route-item\">\n");
        fprintf(file, "            <h2>Start: %s, %s</h2>\n", city->capital, city->country);
        fprintf(file, "            <p>Coordinates: %.6f, %.6f</p>\n", city->lat, city->lon);
        fprintf(file, "        </div>\n");
        push(cities, city);
    }
    
    // Print routes and destinations
    while (!isEmpty(tempRoutes) && !isEmpty(tempCities)) {
        Route* route = (Route*)pop(tempRoutes);
        Location* city = (Location*)pop(tempCities);
        
        fprintf(file, "        <div class=\"route-item\">\n");
        fprintf(file, "            <h3>Travel to: %s, %s</h3>\n", city->capital, city->country);
        fprintf(file, "            <p>Transport: %s</p>\n", route->transport);
        fprintf(file, "            <p>Time: %.2f hours</p>\n", route->time);
        fprintf(file, "            <p>Cost: $%.2f</p>\n", route->cost);
        if (strlen(route->note) > 0) {
            fprintf(file, "            <p>Note: %s</p>\n", route->note);
        }
        fprintf(file, "            <p>Coordinates: %.6f, %.6f</p>\n", city->lat, city->lon);
        fprintf(file, "        </div>\n");
        
        totalCost += route->cost;
        totalTime += route->time;
        
        push(routes, route);
        push(cities, city);
    }
    
    // Print summary
    fprintf(file, "        <div class=\"summary\">\n");
    fprintf(file, "            <p>Total Cost: $%.2f</p>\n", totalCost);
    fprintf(file, "            <p>Total Time: %.2f hours</p>\n", totalTime);
    fprintf(file, "        </div>\n");
    
    fprintf(file, "    </div>\n");
    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");
    
    fclose(file);
    printf("Output generated to: %s\n", filename);
    
    // Free temporary stacks
    freeStack(tempCities);
    freeStack(tempRoutes);
}

#endif // FILEOPERATIONS_H 