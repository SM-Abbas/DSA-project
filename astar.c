#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Structure to represent a city with coordinates
typedef struct {
    char name[256];
    char country[256];
    double latitude;
    double longitude;
} City;

// Structure to represent a route between cities
typedef struct {
    char from[256];
    char to[256];
    double distance;
    double cost;
    double time;
} Route;

// Structure for A* algorithm node
typedef struct Node {
    char city[256];
    double g_cost;  // Cost from start to current node
    double h_cost;  // Heuristic cost to goal
    double f_cost;  // g_cost + h_cost
    struct Node* parent;
} Node;

// Structure for priority queue
typedef struct PQNode {
    Node* node;
    struct PQNode* next;
} PQNode;

typedef struct {
    PQNode* front;
} PriorityQueue;

// Function prototypes
City* createCity(const char* name, const char* country, double lat, double lon);
void freeCity(City* city);
Route* createRoute(const char* from, const char* to, double distance, double cost, double time);
void freeRoute(Route* route);
Node* createNode(const char* city, double g_cost, double h_cost, Node* parent);
void freeNode(Node* node);
void freeAllNodes(Node* node);
PriorityQueue* createPriorityQueue();
void freePriorityQueue(PriorityQueue* pq);
void push(PriorityQueue* pq, Node* node);
Node* pop(PriorityQueue* pq);
int isEmpty(PriorityQueue* pq);
double haversine(double lat1, double lon1, double lat2, double lon2);
double heuristic(const City* a, const City* b);
int findCityIndex(City** cities, int cityCount, const char* name);
void parseCitiesFile(const char* filename, City*** cities, int* cityCount);
void parseRoutesFile(const char* filename, Route*** routes, int* routeCount);
void generateOutputFile(const char* filename, Node* path, City** cities, int cityCount, Route** routes, int routeCount, const char* criteria, clock_t startTime, int nodesVisited);
Node* astar(City** cities, int cityCount, Route** routes, int routeCount, const char* start, const char* goal, const char* criteria, int* nodesVisited);

// City functions
City* createCity(const char* name, const char* country, double lat, double lon) {
    City* city = (City*)malloc(sizeof(City));
    if (city == NULL) {
        return NULL;
    }
    
    strncpy(city->name, name, sizeof(city->name) - 1);
    city->name[sizeof(city->name) - 1] = '\0';
    
    strncpy(city->country, country, sizeof(city->country) - 1);
    city->country[sizeof(city->country) - 1] = '\0';
    
    city->latitude = lat;
    city->longitude = lon;
    
    return city;
}

void freeCity(City* city) {
    free(city);
}

// Route functions
Route* createRoute(const char* from, const char* to, double distance, double cost, double time) {
    Route* route = (Route*)malloc(sizeof(Route));
    if (route == NULL) {
        return NULL;
    }
    
    strncpy(route->from, from, sizeof(route->from) - 1);
    route->from[sizeof(route->from) - 1] = '\0';
    
    strncpy(route->to, to, sizeof(route->to) - 1);
    route->to[sizeof(route->to) - 1] = '\0';
    
    route->distance = distance;
    route->cost = cost;
    route->time = time;
    
    return route;
}

void freeRoute(Route* route) {
    free(route);
}

// Node functions
Node* createNode(const char* city, double g_cost, double h_cost, Node* parent) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        return NULL;
    }
    
    strncpy(node->city, city, sizeof(node->city) - 1);
    node->city[sizeof(node->city) - 1] = '\0';
    
    node->g_cost = g_cost;
    node->h_cost = h_cost;
    node->f_cost = g_cost + h_cost;
    node->parent = parent;
    
    return node;
}

void freeNode(Node* node) {
    free(node);
}

void freeAllNodes(Node* node) {
    if (node == NULL) {
        return;
    }
    
    if (node->parent != NULL) {
        freeAllNodes(node->parent);
    }
    
    freeNode(node);
}

// Priority queue functions
PriorityQueue* createPriorityQueue() {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if (pq == NULL) {
        return NULL;
    }
    
    pq->front = NULL;
    
    return pq;
}

void freePriorityQueue(PriorityQueue* pq) {
    if (pq == NULL) {
        return;
    }
    
    PQNode* current = pq->front;
    while (current != NULL) {
        PQNode* next = current->next;
        free(current);
        current = next;
    }
    
    free(pq);
}

void push(PriorityQueue* pq, Node* node) {
    if (pq == NULL || node == NULL) {
        return;
    }
    
    PQNode* newNode = (PQNode*)malloc(sizeof(PQNode));
    if (newNode == NULL) {
        return;
    }
    
    newNode->node = node;
    
    // Insert in sorted order (by f_cost)
    if (pq->front == NULL || pq->front->node->f_cost > node->f_cost) {
        newNode->next = pq->front;
        pq->front = newNode;
    } else {
        PQNode* current = pq->front;
        while (current->next != NULL && current->next->node->f_cost <= node->f_cost) {
            current = current->next;
        }
        
        newNode->next = current->next;
        current->next = newNode;
    }
}

Node* pop(PriorityQueue* pq) {
    if (pq == NULL || pq->front == NULL) {
        return NULL;
    }
    
    PQNode* frontNode = pq->front;
    Node* node = frontNode->node;
    
    pq->front = frontNode->next;
    free(frontNode);
    
    return node;
}

int isEmpty(PriorityQueue* pq) {
    return pq == NULL || pq->front == NULL;
}

// Haversine formula to calculate distance between two points on Earth
double haversine(double lat1, double lon1, double lat2, double lon2) {
    // Convert latitude and longitude from degrees to radians
    lat1 = lat1 * M_PI / 180.0;
    lon1 = lon1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    lon2 = lon2 * M_PI / 180.0;
    
    // Haversine formula
    double dlon = lon2 - lon1;
    double dlat = lat2 - lat1;
    double a = sin(dlat/2) * sin(dlat/2) + cos(lat1) * cos(lat2) * sin(dlon/2) * sin(dlon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    // Earth radius in kilometers
    double radius = 6371.0;
    
    return radius * c;
}

// Heuristic function for A* (straight-line distance)
double heuristic(const City* a, const City* b) {
    return haversine(a->latitude, a->longitude, b->latitude, b->longitude);
}

// Find city index by name
int findCityIndex(City** cities, int cityCount, const char* name) {
    for (int i = 0; i < cityCount; i++) {
        if (strcmp(cities[i]->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Parse cities from file
void parseCitiesFile(const char* filename, City*** cities, int* cityCount) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening cities file: %s\n", filename);
        return;
    }
    
    // Count lines to allocate memory
    int count = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    
    // Allocate memory for cities
    *cities = (City**)malloc(count * sizeof(City*));
    if (*cities == NULL) {
        fclose(file);
        return;
    }
    
    // Reset file pointer to beginning
    rewind(file);
    
    // Parse file
    *cityCount = 0;
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse CSV line
        char* token = strtok(line, ",");
        if (token == NULL) continue;
        char country[256];
        strncpy(country, token, sizeof(country) - 1);
        country[sizeof(country) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        char name[256];
        strncpy(name, token, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        double lat = atof(token);
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        double lon = atof(token);
        
        // Create city
        City* city = createCity(name, country, lat, lon);
        if (city != NULL) {
            (*cities)[(*cityCount)++] = city;
        }
    }
    
    fclose(file);
}

// Parse routes from file
void parseRoutesFile(const char* filename, Route*** routes, int* routeCount) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening routes file: %s\n", filename);
        return;
    }
    
    // Count lines to allocate memory
    int count = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    
    // Allocate memory for routes
    *routes = (Route**)malloc(count * sizeof(Route*));
    if (*routes == NULL) {
        fclose(file);
        return;
    }
    
    // Reset file pointer to beginning
    rewind(file);
    
    // Parse file
    *routeCount = 0;
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse CSV line
        char* token = strtok(line, ",");
        if (token == NULL) continue;
        char from[256];
        strncpy(from, token, sizeof(from) - 1);
        from[sizeof(from) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        char to[256];
        strncpy(to, token, sizeof(to) - 1);
        to[sizeof(to) - 1] = '\0';
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        // Skip transport mode
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        double time = atof(token);
        
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        double cost = atof(token);
        
        // Calculate distance (will be recalculated based on city coordinates)
        double distance = 0.0;
        
        // Create route
        Route* route = createRoute(from, to, distance, cost, time);
        if (route != NULL) {
            (*routes)[(*routeCount)++] = route;
        }
    }
    
    fclose(file);
}

// Generate output HTML file
void generateOutputFile(const char* filename, Node* path, City** cities, int cityCount, Route** routes, int routeCount, const char* criteria, clock_t startTime, int nodesVisited) {
    if (path == NULL) {
        printf("No path found.\n");
        return;
    }
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening output file: %s\n", filename);
        return;
    }
    
    // Calculate execution time
    clock_t endTime = clock();
    double executionTime = ((double)(endTime - startTime)) / CLOCKS_PER_SEC * 1000.0; // in milliseconds
    
    fprintf(file, "<!DOCTYPE html>\n");
    fprintf(file, "<html>\n");
    fprintf(file, "<head>\n");
    fprintf(file, "    <title>A* Algorithm Route</title>\n");
    fprintf(file, "    <style>\n");
    fprintf(file, "        body { font-family: Arial, sans-serif; margin: 20px; }\n");
    fprintf(file, "        h1 { color: #2c3e50; }\n");
    fprintf(file, "        .route { margin-bottom: 20px; }\n");
    fprintf(file, "        .route-item { margin: 10px 0; padding: 10px; border: 1px solid #ddd; border-radius: 5px; }\n");
    fprintf(file, "        .summary { font-weight: bold; margin-top: 20px; background-color: #f8f9fa; padding: 15px; border-radius: 5px; }\n");
    fprintf(file, "        .stats { margin-top: 20px; background-color: #e9f7ef; padding: 15px; border-radius: 5px; }\n");
    fprintf(file, "    </style>\n");
    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");
    fprintf(file, "    <h1>A* Algorithm Route (%s)</h1>\n", criteria);
    fprintf(file, "    <div class=\"route\">\n");
    
    // Count path length and build path array
    int pathLength = 0;
    Node* current = path;
    while (current != NULL) {
        pathLength++;
        current = current->parent;
    }
    
    char** pathArray = (char**)malloc(pathLength * sizeof(char*));
    if (pathArray == NULL) {
        fclose(file);
        return;
    }
    
    current = path;
    for (int i = pathLength - 1; i >= 0; i--) {
        pathArray[i] = current->city;
        current = current->parent;
    }
    
    // Calculate totals
    double totalCost = 0.0;
    double totalTime = 0.0;
    double totalDistance = 0.0;
    
    // Print route
    for (int i = 0; i < pathLength; i++) {
        int cityIndex = findCityIndex(cities, cityCount, pathArray[i]);
        if (cityIndex == -1) continue;
        
        City* city = cities[cityIndex];
        
        fprintf(file, "        <div class=\"route-item\">\n");
        
        if (i == 0) {
            fprintf(file, "            <h2>Start: %s, %s</h2>\n", city->name, city->country);
        } else {
            fprintf(file, "            <h2>To: %s, %s</h2>\n", city->name, city->country);
            
            // Find route info
            for (int j = 0; j < routeCount; j++) {
                if (strcmp(routes[j]->from, pathArray[i-1]) == 0 && strcmp(routes[j]->to, pathArray[i]) == 0) {
                    fprintf(file, "            <p>Time: %.2f hours</p>\n", routes[j]->time);
                    fprintf(file, "            <p>Cost: $%.2f</p>\n", routes[j]->cost);
                    
                    int prevCityIndex = findCityIndex(cities, cityCount, pathArray[i-1]);
                    if (prevCityIndex != -1) {
                        double dist = haversine(cities[prevCityIndex]->latitude, cities[prevCityIndex]->longitude, 
                                              city->latitude, city->longitude);
                        fprintf(file, "            <p>Distance: %.2f km</p>\n", dist);
                        
                        totalDistance += dist;
                        totalCost += routes[j]->cost;
                        totalTime += routes[j]->time;
                    }
                    
                    break;
                }
            }
        }
        
        fprintf(file, "            <p>Coordinates: %.6f, %.6f</p>\n", city->latitude, city->longitude);
        fprintf(file, "        </div>\n");
    }
    
    // Print summary
    fprintf(file, "        <div class=\"summary\">\n");
    fprintf(file, "            <h2>Route Summary</h2>\n");
    fprintf(file, "            <p>Total Distance: %.2f km</p>\n", totalDistance);
    fprintf(file, "            <p>Total Cost: $%.2f</p>\n", totalCost);
    fprintf(file, "            <p>Total Time: %.2f hours</p>\n", totalTime);
    fprintf(file, "        </div>\n");
    
    // Print algorithm stats
    fprintf(file, "        <div class=\"stats\">\n");
    fprintf(file, "            <h2>Algorithm Statistics</h2>\n");
    fprintf(file, "            <p>Execution Time: %.2f ms</p>\n", executionTime);
    fprintf(file, "            <p>Nodes Visited: %d</p>\n", nodesVisited);
    fprintf(file, "            <p>Path Length: %d cities</p>\n", pathLength);
    fprintf(file, "        </div>\n");
    
    fprintf(file, "    </div>\n");
    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");
    
    fclose(file);
    free(pathArray);
    
    printf("Output generated to: %s\n", filename);
}

// A* algorithm implementation
Node* astar(City** cities, int cityCount, Route** routes, int routeCount, const char* start, const char* goal, const char* criteria, int* nodesVisited) {
    if (cities == NULL || cityCount <= 0 || routes == NULL || routeCount <= 0) {
        return NULL;
    }
    
    int startIndex = findCityIndex(cities, cityCount, start);
    int goalIndex = findCityIndex(cities, cityCount, goal);
    
    if (startIndex == -1 || goalIndex == -1) {
        printf("Start or goal city not found.\n");
        return NULL;
    }
    
    // Initialize priority queue and visited set
    PriorityQueue* openSet = createPriorityQueue();
    if (openSet == NULL) {
        return NULL;
    }
    
    char** closedSet = (char**)malloc(cityCount * sizeof(char*));
    if (closedSet == NULL) {
        freePriorityQueue(openSet);
        return NULL;
    }
    int closedSetSize = 0;
    
    // Initialize start node
    double h_cost = heuristic(cities[startIndex], cities[goalIndex]);
    Node* startNode = createNode(start, 0.0, h_cost, NULL);
    if (startNode == NULL) {
        freePriorityQueue(openSet);
        free(closedSet);
        return NULL;
    }
    
    push(openSet, startNode);
    *nodesVisited = 0;
    
    // A* algorithm
    while (!isEmpty(openSet)) {
        Node* current = pop(openSet);
        (*nodesVisited)++;
        
        // Check if goal reached
        if (strcmp(current->city, goal) == 0) {
            freePriorityQueue(openSet);
            free(closedSet);
            return current;
        }
        
        // Add to closed set
        closedSet[closedSetSize++] = current->city;
        
        // Check neighbors
        for (int i = 0; i < routeCount; i++) {
            if (strcmp(routes[i]->from, current->city) == 0) {
                const char* neighbor = routes[i]->to;
                
                // Check if in closed set
                int inClosedSet = 0;
                for (int j = 0; j < closedSetSize; j++) {
                    if (strcmp(closedSet[j], neighbor) == 0) {
                        inClosedSet = 1;
                        break;
                    }
                }
                
                if (inClosedSet) {
                    continue;
                }
                
                // Calculate cost based on criteria
                double cost = 0.0;
                if (strcmp(criteria, "cost") == 0) {
                    cost = routes[i]->cost;
                } else { // Default to time
                    cost = routes[i]->time;
                }
                
                double g_cost = current->g_cost + cost;
                
                int neighborIndex = findCityIndex(cities, cityCount, neighbor);
                if (neighborIndex == -1) {
                    continue;
                }
                
                double h_cost = heuristic(cities[neighborIndex], cities[goalIndex]);
                Node* neighborNode = createNode(neighbor, g_cost, h_cost, current);
                
                push(openSet, neighborNode);
            }
        }
    }
    
    // No path found
    freePriorityQueue(openSet);
    free(closedSet);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        printf("Usage: %s <cities_file> <routes_file> <start_city> <end_city> [criteria] [output_file]\n", argv[0]);
        return 1;
    }
    
    const char* citiesFile = argv[1];
    const char* routesFile = argv[2];
    const char* startCity = argv[3];
    const char* endCity = argv[4];
    const char* criteria = (argc > 5) ? argv[5] : "time"; // Default to time if not specified
    const char* outputFile = (argc > 6) ? argv[6] : "astar_output.html"; // Default output file
    
    // Parse input files
    City** cities = NULL;
    int cityCount = 0;
    Route** routes = NULL;
    int routeCount = 0;
    
    parseCitiesFile(citiesFile, &cities, &cityCount);
    parseRoutesFile(routesFile, &routes, &routeCount);
    
    if (cities == NULL || cityCount == 0 || routes == NULL || routeCount == 0) {
        printf("Error parsing input files.\n");
        return 1;
    }
    
    // Calculate distances for routes
    for (int i = 0; i < routeCount; i++) {
        int fromIndex = findCityIndex(cities, cityCount, routes[i]->from);
        int toIndex = findCityIndex(cities, cityCount, routes[i]->to);
        
        if (fromIndex != -1 && toIndex != -1) {
            routes[i]->distance = haversine(cities[fromIndex]->latitude, cities[fromIndex]->longitude,
                                          cities[toIndex]->latitude, cities[toIndex]->longitude);
        }
    }
    
    // Run A* algorithm
    clock_t startTime = clock();
    int nodesVisited = 0;
    Node* path = astar(cities, cityCount, routes, routeCount, startCity, endCity, criteria, &nodesVisited);
    
    // Generate output
    generateOutputFile(outputFile, path, cities, cityCount, routes, routeCount, criteria, startTime, nodesVisited);
    
    // Cleanup
    freeAllNodes(path);
    
    for (int i = 0; i < cityCount; i++) {
        freeCity(cities[i]);
    }
    free(cities);
    
    for (int i = 0; i < routeCount; i++) {
        freeRoute(routes[i]);
    }
    free(routes);
    
    return 0;
} 