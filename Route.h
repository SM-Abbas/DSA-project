#ifndef ROUTE_H
#define ROUTE_H

#include <stdlib.h>
#include <string.h>

// Forward declaration
struct Location;
typedef struct Location Location;

#define MULTI 3.0f

// Route structure
typedef struct Route {
	Location* origin;
	Location* destination;

	char originS[256];
	char destinationS[256];

	char transport[256];
	float time;
	float cost;
	char note[256];
} Route;

// Function prototypes
Route* createRoute();
void freeRoute(Route* route);
Route* createRouteWithLocations(Location* org, Location* dest);
Route* createRouteWithDetails(Location* org, Location* dest, const char* trans, float tim, float cst, const char* notee);
int doesRouteConnect(Route* route, Location* start, Location* end);

// Implementation
Route* createRoute() {
	Route* route = (Route*)malloc(sizeof(Route));
	if (route == NULL) {
		return NULL;
	}
	
	route->origin = NULL;
	route->destination = NULL;
	route->originS[0] = '\0';
	route->destinationS[0] = '\0';
	route->transport[0] = '\0';
	route->time = 0;
	route->cost = 0;
	route->note[0] = '\0';
	
	return route;
}

void freeRoute(Route* route) {
	if (route == NULL) {
		return;
	}
	
	// Don't free origin and destination, they should be freed by the graph
	free(route);
}

Route* createRouteWithLocations(Location* org, Location* dest) {
	Route* route = createRoute();
	if (route == NULL) {
		return NULL;
	}
	
	route->origin = org;
	route->destination = dest;
	
	return route;
}

Route* createRouteWithDetails(Location* org, Location* dest, const char* trans, float tim, float cst, const char* notee) {
	Route* route = createRouteWithLocations(org, dest);
	if (route == NULL) {
		return NULL;
	}
	
	strncpy(route->transport, trans, 255);
	route->transport[255] = '\0';
	
	route->time = tim;
	route->cost = cst;
	
	strncpy(route->note, notee, 255);
	route->note[255] = '\0';
	
	return route;
}

int doesRouteConnect(Route* route, Location* start, Location* end) {
	if (route == NULL || start == NULL || end == NULL) {
		return 0;
	}
	
	if (route->origin == start && route->destination == end) {
		return 1;
	}
	
	return 0;
}

#endif // ROUTE_H
