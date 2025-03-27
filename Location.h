#ifndef LOCATION_H
#define LOCATION_H

#include <stdlib.h>
#include <string.h>

// Forward declaration
struct Route;
typedef struct Route Route;

// Location structure
typedef struct Location {
	char country[256];
	char capital[256];
	float lat;
	float lon;

	Route** routes;
	int routeCount;
	int routeCapacity;

	int exists;
	struct Location* previous;
	float lengthFromStart;
} Location;

// Function prototypes
Location* createLocation();
void freeLocation(Location* loc);
Location* createLocationWithData(const char* count, const char* cap);
Location* createLocationWithCoords(const char* count, const char* cap, float lt, float lg);
float getLocationWeight(Location* loc, Location* start, Location* end);
int compareLocations(const Location* l1, const Location* l2);
int locationLessThan(const Location* l1, const Location* l2);
int locationGreaterThan(const Location* l1, const Location* l2);
int locationCompare(Location* l1, Location* l2);
void addRouteToLocation(Location* loc, Route* route);

// Implementation
Location* createLocation() {
	Location* loc = (Location*)malloc(sizeof(Location));
	if (loc == NULL) {
		return NULL;
	}
	
	loc->country[0] = '\0';
	loc->capital[0] = '\0';
	loc->lat = 0;
	loc->lon = 0;

	// Used as a highest value possible for comparison purposes
	loc->lengthFromStart = 999999;

	loc->exists = 1;
	loc->previous = NULL;
	
	loc->routes = NULL;
	loc->routeCount = 0;
	loc->routeCapacity = 0;
	
	return loc;
}

void freeLocation(Location* loc) {
	if (loc == NULL) {
		return;
	}
	
	// Don't free the routes here, they should be freed by the graph
	free(loc->routes);
	free(loc);
}

Location* createLocationWithData(const char* count, const char* cap) {
	Location* loc = createLocation();
	if (loc == NULL) {
		return NULL;
	}
	
	strncpy(loc->country, count, 255);
	loc->country[255] = '\0';
	
	strncpy(loc->capital, cap, 255);
	loc->capital[255] = '\0';
	
	return loc;
}

Location* createLocationWithCoords(const char* count, const char* cap, float lt, float lg) {
	Location* loc = createLocationWithData(count, cap);
	if (loc == NULL) {
		return NULL;
	}
	
	loc->lat = lt;
	loc->lon = lg;
	
	return loc;
}

int compareLocations(const Location* l1, const Location* l2) {
	return strcmp(l1->capital, l2->capital);
}

int locationLessThan(const Location* l1, const Location* l2) {
	return l1->lengthFromStart < l2->lengthFromStart;
}

int locationGreaterThan(const Location* l1, const Location* l2) {
	return l1->lengthFromStart > l2->lengthFromStart;
}

int locationCompare(Location* l1, Location* l2) {
	return l1->lengthFromStart < l2->lengthFromStart;
}

void addRouteToLocation(Location* loc, Route* route) {
	if (loc == NULL || route == NULL) {
		return;
	}
	
	// Resize array if needed
	if (loc->routeCount >= loc->routeCapacity) {
		int newCapacity = loc->routeCapacity == 0 ? 4 : loc->routeCapacity * 2;
		Route** newRoutes = (Route**)realloc(loc->routes, newCapacity * sizeof(Route*));
		if (newRoutes == NULL) {
			return;
		}
		loc->routes = newRoutes;
		loc->routeCapacity = newCapacity;
	}
	
	loc->routes[loc->routeCount++] = route;
}

#endif // LOCATION_H
