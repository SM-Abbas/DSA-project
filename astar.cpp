#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <limits>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Structure to represent a city with coordinates
struct City {
    std::string name;
    std::string country;
    double latitude;
    double longitude;
    
    City() {}
    
    City(const std::string& n, const std::string& c, double lat, double lon)
        : name(n), country(c), latitude(lat), longitude(lon) {}
};

// Structure to represent a route between cities
struct Route {
    std::string from;
    std::string to;
    double distance;
    double cost;
    double time;
    
    Route() {}
    
    Route(const std::string& f, const std::string& t, double d, double c, double tm)
        : from(f), to(t), distance(d), cost(c), time(tm) {}
};

// Structure for A* algorithm node
struct Node {
    std::string city;
    double g_cost; // Cost from start to current node
    double h_cost; // Heuristic cost (estimated cost from current to goal)
    double f_cost; // Total cost (g_cost + h_cost)
    std::string parent;
    
    Node() : g_cost(0), h_cost(0), f_cost(0) {}
    
    Node(const std::string& c, double g, double h, const std::string& p)
        : city(c), g_cost(g), h_cost(h), f_cost(g + h), parent(p) {}
    
    // Comparison operator for priority queue
    bool operator>(const Node& other) const {
        return f_cost > other.f_cost;
    }
};

// Class for travel planning using A* algorithm
class TravelPlanner {
private:
    std::unordered_map<std::string, City> cities;
    std::unordered_map<std::string, std::vector<Route>> routes;
    int nodesVisited;
    double computationTime;
    
    // Calculate Haversine distance between two points on Earth
    double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
        const double R = 6371.0; // Earth radius in kilometers
        
        double dLat = (lat2 - lat1) * M_PI / 180.0;
        double dLon = (lon2 - lon1) * M_PI / 180.0;
        
        double a = sin(dLat/2) * sin(dLat/2) +
                  cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
                  sin(dLon/2) * sin(dLon/2);
        
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        return R * c;
    }
    
    // Calculate heuristic (straight-line distance)
    double calculateHeuristic(const std::string& from, const std::string& to) {
        if (cities.find(from) == cities.end() || cities.find(to) == cities.end()) {
            return 0.0;
        }
        
        const City& fromCity = cities[from];
        const City& toCity = cities[to];
        
        return haversineDistance(fromCity.latitude, fromCity.longitude, 
                                toCity.latitude, toCity.longitude);
    }
    
public:
    TravelPlanner() : nodesVisited(0), computationTime(0.0) {}
    
    // Load cities from CSV file
    bool loadCities(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open cities file: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        // Skip header line
        std::getline(file, line);
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string country, city, lat_str, lon_str;
            
            // Parse CSV line
            std::getline(ss, country, ',');
            std::getline(ss, city, ',');
            std::getline(ss, lat_str, ',');
            std::getline(ss, lon_str, ',');
            
            try {
                double latitude = std::stod(lat_str);
                double longitude = std::stod(lon_str);
                
                cities[city] = City(city, country, latitude, longitude);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing city data: " << line << std::endl;
            }
        }
        
        file.close();
        return true;
    }
    
    // Generate routes between cities
    void generateRoutes() {
        // For each city, create routes to other cities
        for (const auto& from : cities) {
            for (const auto& to : cities) {
                if (from.first != to.first) {
                    double distance = haversineDistance(from.second.latitude, from.second.longitude,
                                                      to.second.latitude, to.second.longitude);
                    
                    // Simulate cost and time based on distance
                    double cost = distance * (0.5 + ((double)rand() / RAND_MAX) * 0.5); // Random factor for cost
                    double time = distance / 800.0 * (0.8 + ((double)rand() / RAND_MAX) * 0.4); // Assume average speed of 800 km/h with random factor
                    
                    routes[from.first].push_back(Route(from.first, to.first, distance, cost, time));
                }
            }
        }
    }
    
    // Find route using A* algorithm
    std::vector<Route> findRoute(const std::string& start, const std::string& goal, const std::string& preference) {
        auto startTime = std::chrono::high_resolution_clock::now();
        nodesVisited = 0;
        
        // Check if cities exist
        if (cities.find(start) == cities.end() || cities.find(goal) == cities.end()) {
            std::cerr << "Error: Start or goal city not found." << std::endl;
            return {};
        }
        
        // Priority queue for A* algorithm
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_set<std::string> closedSet;
        std::unordered_map<std::string, Node> allNodes;
        
        // Initialize start node
        Node startNode(start, 0, calculateHeuristic(start, goal), "");
        openSet.push(startNode);
        allNodes[start] = startNode;
        
        while (!openSet.empty()) {
            // Get node with lowest f_cost
            Node current = openSet.top();
            openSet.pop();
            nodesVisited++;
            
            // If goal reached, reconstruct path
            if (current.city == goal) {
                auto endTime = std::chrono::high_resolution_clock::now();
                computationTime = std::chrono::duration<double>(endTime - startTime).count();
                
                return reconstructPath(allNodes, start, goal);
            }
            
            // Add to closed set
            closedSet.insert(current.city);
            
            // Check all neighbors
            for (const Route& route : routes[current.city]) {
                // Skip if neighbor is in closed set
                if (closedSet.find(route.to) != closedSet.end()) {
                    continue;
                }
                
                // Calculate cost based on preference
                double edgeCost;
                if (preference == "fastest") {
                    edgeCost = route.time;
                } else if (preference == "cheapest") {
                    edgeCost = route.cost;
                } else {
                    edgeCost = route.distance;
                }
                
                double tentative_g = current.g_cost + edgeCost;
                
                // If neighbor not in open set or better path found
                if (allNodes.find(route.to) == allNodes.end() || tentative_g < allNodes[route.to].g_cost) {
                    // Update node
                    Node neighbor(route.to, tentative_g, calculateHeuristic(route.to, goal), current.city);
                    allNodes[route.to] = neighbor;
                    
                    // Add to open set
                    openSet.push(neighbor);
                }
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        computationTime = std::chrono::duration<double>(endTime - startTime).count();
        
        // No path found
        return {};
    }
    
    // Reconstruct path from A* result
    std::vector<Route> reconstructPath(const std::unordered_map<std::string, Node>& nodes, 
                                     const std::string& start, const std::string& goal) {
        std::vector<Route> path;
        std::string current = goal;
        
        while (current != start) {
            const Node& currentNode = nodes.at(current);
            const std::string& parent = currentNode.parent;
            
            // Find the route from parent to current
            for (const Route& route : routes[parent]) {
                if (route.to == current) {
                    path.push_back(route);
                    break;
                }
            }
            
            current = parent;
        }
        
        // Reverse path to get start to goal
        std::reverse(path.begin(), path.end());
        return path;
    }
    
    // Get statistics
    int getNodesVisited() const {
        return nodesVisited;
    }
    
    double getComputationTime() const {
        return computationTime;
    }
    
    // Print route details
    void printRoute(const std::vector<Route>& route) {
        if (route.empty()) {
            std::cout << "No route found." << std::endl;
            return;
        }
        
        double totalDistance = 0.0;
        double totalCost = 0.0;
        double totalTime = 0.0;
        
        std::cout << "Route from " << route.front().from << " to " << route.back().to << ":" << std::endl;
        
        for (const Route& segment : route) {
            std::cout << "  " << segment.from << " -> " << segment.to 
                      << " (Distance: " << std::fixed << std::setprecision(2) << segment.distance << " km, "
                      << "Cost: $" << std::fixed << std::setprecision(2) << segment.cost << ", "
                      << "Time: " << std::fixed << std::setprecision(2) << segment.time << " hours)" << std::endl;
            
            totalDistance += segment.distance;
            totalCost += segment.cost;
            totalTime += segment.time;
        }
        
        std::cout << "Total Distance: " << std::fixed << std::setprecision(2) << totalDistance << " km" << std::endl;
        std::cout << "Total Cost: $" << std::fixed << std::setprecision(2) << totalCost << std::endl;
        std::cout << "Total Time: " << std::fixed << std::setprecision(2) << totalTime << " hours" << std::endl;
        std::cout << "Nodes Visited: " << nodesVisited << std::endl;
        std::cout << "Computation Time: " << std::fixed << std::setprecision(6) << computationTime << " seconds" << std::endl;
    }
    
    // Output route as JSON
    std::string routeToJson(const std::vector<Route>& route) {
        if (route.empty()) {
            return "{\"error\": \"No route found.\"}";
        }
        
        double totalDistance = 0.0;
        double totalCost = 0.0;
        double totalTime = 0.0;
        
        std::stringstream json;
        json << "{";
        json << "\"origin\": \"" << route.front().from << "\",";
        json << "\"destination\": \"" << route.back().to << "\",";
        json << "\"steps\": [";
        
        for (size_t i = 0; i < route.size(); ++i) {
            const Route& segment = route[i];
            
            json << "{";
            json << "\"from\": \"" << segment.from << "\",";
            json << "\"to\": \"" << segment.to << "\",";
            json << "\"distance\": " << std::fixed << std::setprecision(2) << segment.distance << ",";
            json << "\"cost\": " << std::fixed << std::setprecision(2) << segment.cost << ",";
            json << "\"time\": " << std::fixed << std::setprecision(2) << segment.time;
            json << "}";
            
            if (i < route.size() - 1) {
                json << ",";
            }
            
            totalDistance += segment.distance;
            totalCost += segment.cost;
            totalTime += segment.time;
        }
        
        json << "],";
        json << "\"totalDistance\": " << std::fixed << std::setprecision(2) << totalDistance << ",";
        json << "\"totalCost\": " << std::fixed << std::setprecision(2) << totalCost << ",";
        json << "\"totalTime\": " << std::fixed << std::setprecision(2) << totalTime << ",";
        json << "\"nodesVisited\": " << nodesVisited << ",";
        json << "\"computationTime\": " << std::fixed << std::setprecision(6) << computationTime;
        json << "}";
        
        return json.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <cities_file> <origin> <destination> [preference]" << std::endl;
        std::cerr << "Preference can be 'fastest' or 'cheapest' (default: fastest)" << std::endl;
        return 1;
    }
    
    std::string citiesFile = argv[1];
    std::string origin = argv[2];
    std::string destination = argv[3];
    std::string preference = (argc > 4) ? argv[4] : "fastest";
    
    // Seed random number generator
    srand(static_cast<unsigned int>(time(nullptr)));
    
    TravelPlanner planner;
    
    // Load cities
    if (!planner.loadCities(citiesFile)) {
        return 1;
    }
    
    // Generate routes
    planner.generateRoutes();
    
    // Find route
    std::vector<Route> route = planner.findRoute(origin, destination, preference);
    
    // Output as JSON
    std::cout << planner.routeToJson(route) << std::endl;
    
    return 0;
} 