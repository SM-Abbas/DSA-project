// Constants
const API_BASE_URL = window.location.origin;

// Global variables
let dijkstraMap = null;
let astarMap = null;
let cityCoordinates = {};
let cityList = [];
let indianCities = [];

// Initialize the application when the DOM is loaded
document.addEventListener('DOMContentLoaded', function() {
    console.log('DOM fully loaded');
    console.log('API_BASE_URL:', API_BASE_URL);
    
    // Log the presence of key elements
    console.log('Find Route button exists:', !!document.getElementById('findRoute'));
    console.log('Indian toggle exists:', !!document.getElementById('indian-toggle'));
    console.log('Global toggle exists:', !!document.getElementById('global-toggle'));
    console.log('Dijkstra map exists:', !!document.getElementById('dijkstra-map'));
    console.log('A* map exists:', !!document.getElementById('astar-map'));
    console.log('Loading indicator exists:', !!document.getElementById('loading-indicator'));
    
    init();
});

// Initialize maps and event listeners
async function init() {
    console.log('Setting up maps...');
    initMaps();
    
    console.log('Loading city data...');
    await loadCityData();
    
    // Set up event listeners
    document.getElementById('findRoute').addEventListener('click', handleFindRoute);
    document.getElementById('indian-toggle').addEventListener('click', () => toggleRegion('indian'));
    document.getElementById('global-toggle').addEventListener('click', () => toggleRegion('global'));
    
    // Set up autocomplete for origin and destination inputs
    setupAutocomplete('origin');
    setupAutocomplete('destination');
    
    // Set up quick route buttons
    document.querySelectorAll('.quick-route').forEach(button => {
        button.addEventListener('click', handleQuickRoute);
    });
    
    console.log('Initialization complete');
}

// Initialize Google Maps
function initMaps() {
    const mapOptions = {
        center: { lat: 22.5726, lng: 79.3639 },
        zoom: 5,
        styles: [
            {
                featureType: "poi",
                elementType: "labels",
                stylers: [{ visibility: "off" }]
            }
        ]
    };
    
    try {
        // Initialize maps with the same options
        const dijkstraMapElement = document.getElementById('dijkstra-map');
        const astarMapElement = document.getElementById('astar-map');
        
        if (dijkstraMapElement) {
            dijkstraMap = new google.maps.Map(dijkstraMapElement, mapOptions);
        } else {
            console.error('Dijkstra map element not found');
        }
        
        if (astarMapElement) {
            astarMap = new google.maps.Map(astarMapElement, mapOptions);
        } else {
            console.error('A* map element not found');
        }
        
        console.log('Maps initialized successfully:', {
            dijkstraMap: !!dijkstraMap,
            astarMap: !!astarMap
        });
    } catch (error) {
        console.error('Error initializing maps:', error);
    }
}

// Load city data from the server
async function loadCityData() {
    try {
        console.log('Fetching city data...');
        const [citiesResponse, indianCitiesResponse, coordinatesResponse] = await Promise.all([
            fetch(`${API_BASE_URL}/get-cities`),
            fetch(`${API_BASE_URL}/get-indian-cities`),
            fetch(`${API_BASE_URL}/get-cities-coordinates`)
        ]);

        const cities = await citiesResponse.json();
        const indian = await indianCitiesResponse.json();
        const coordinates = await coordinatesResponse.json();

        cityList = cities;
        indianCities = indian;
        cityCoordinates = coordinates;
        
        console.log('City data loaded successfully');
    } catch (error) {
        console.error('Error loading city data:', error);
        alert('Error loading city data. Please refresh the page.');
    }
}

// Set up autocomplete for input fields
function setupAutocomplete(inputId) {
    const input = document.getElementById(inputId);
    const resultsContainer = document.getElementById(`${inputId}-autocomplete`);
    
    input.addEventListener('input', () => {
        const value = input.value.toLowerCase();
        const results = filterCities(value);
        displayAutocompleteResults(results, resultsContainer, input);
    });
    
    input.addEventListener('focus', () => {
        if (input.value) {
            const results = filterCities(input.value.toLowerCase());
            displayAutocompleteResults(results, resultsContainer, input);
        }
    });
    
    // Close autocomplete when clicking outside
    document.addEventListener('click', (e) => {
        if (!input.contains(e.target) && !resultsContainer.contains(e.target)) {
            resultsContainer.style.display = 'none';
        }
    });
}

// Filter cities based on input
function filterCities(query) {
    const cities = document.getElementById('indian-toggle').classList.contains('active') 
        ? indianCities 
        : cityList;
    
    return cities.filter(city => 
        city.toLowerCase().includes(query)
    ).slice(0, 5); // Limit to 5 results
}

// Display autocomplete results
function displayAutocompleteResults(results, container, input) {
    container.innerHTML = '';
    
    if (results.length > 0) {
        results.forEach(city => {
            const div = document.createElement('div');
            div.textContent = city;
            div.addEventListener('click', () => {
                input.value = city;
                container.style.display = 'none';
            });
            container.appendChild(div);
        });
        container.style.display = 'block';
    } else {
        container.style.display = 'none';
    }
}

// Handle region toggle
function toggleRegion(region) {
    const indianToggle = document.getElementById('indian-toggle');
    const globalToggle = document.getElementById('global-toggle');
    
    if (region === 'indian') {
        indianToggle.classList.add('active');
        globalToggle.classList.remove('active');
    } else {
        indianToggle.classList.remove('active');
        globalToggle.classList.add('active');
    }
    
    // Clear inputs
    document.getElementById('origin').value = '';
    document.getElementById('destination').value = '';
}

// Handle quick route selection
function handleQuickRoute(event) {
    const [origin, destination] = event.target.textContent.split(' to ');
    document.getElementById('origin').value = origin;
    document.getElementById('destination').value = destination;
}

// Handle find route button click
function handleFindRoute() {
    const origin = document.getElementById('origin').value;
    const destination = document.getElementById('destination').value;
    const preference = document.querySelector('input[name="preference"]:checked').value;
    
    if (!origin || !destination) {
        alert('Please select both origin and destination cities.');
        return;
    }
    
    if (!cityList.includes(origin) || !cityList.includes(destination)) {
        alert('Please select valid cities from the suggestions.');
        return;
    }
    
    // Show loading indicator
    const loadingIndicator = document.getElementById('loading-indicator');
    if (loadingIndicator) {
        loadingIndicator.style.display = 'block';
    } else {
        console.warn('Loading indicator element not found');
    }
    
    // Clear previous routes
    clearRoutes();
    
    // Show the results section
    const resultsSection = document.querySelector('.results');
    resultsSection.classList.add('active');
    
    // Scroll to results
    resultsSection.scrollIntoView({ behavior: 'smooth' });
    
    // Compare algorithms
    compareAlgorithms(origin, destination, preference);
}

function compareAlgorithms(origin, destination, preference) {
    console.log('Comparing algorithms...', { origin, destination, preference });
    
    // Clear previous routes
    clearRoutes();
    
    // Call the server to compare algorithms
    fetch(`${API_BASE_URL}/compare-algorithms`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            origin: origin,
            destination: destination,
            preference: preference
        })
    })
    .then(response => {
        if (!response.ok) {
            throw new Error(`Server responded with status: ${response.status}`);
        }
        return response.json();
    })
    .then(data => {
        console.log('Received algorithm comparison results:', data);
        
        // Hide loading indicator
        const loadingIndicator = document.getElementById('loading-indicator');
        if (loadingIndicator) {
            loadingIndicator.style.display = 'none';
        }
        
        // Validate the data structure
        if (!data || !data.dijkstra || !data.astar) {
            console.error('Invalid response format:', data);
            alert('Invalid response format from server');
            return;
        }
        
        // Ensure minimum path data exists
        if (!data.dijkstra.path) {
            data.dijkstra.path = [origin, destination];
        }
        
        if (!data.astar.path) {
            data.astar.path = [origin, destination];  
        }
        
        // Display routes on maps if maps are available
        if (dijkstraMap) {
            displayRouteOnMap(data.dijkstra, dijkstraMap);
        }
        
        if (astarMap) {
            displayRouteOnMap(data.astar, astarMap);
        }
        
        // Display route details
        displayRouteDetails(data.dijkstra);
        
        // Display algorithm comparison
        displayAlgorithmComparison(data.dijkstra, data.astar);
    })
    .catch(error => {
        console.error('Error comparing algorithms:', error);
        
        // Hide loading indicator
        const loadingIndicator = document.getElementById('loading-indicator');
        if (loadingIndicator) {
            loadingIndicator.style.display = 'none';
        }
        
        // Show error message
        alert('An error occurred while comparing algorithms. Please try again.');
    });
}

// Clear existing routes from maps
function clearRoutes() {
    try {
        console.log('Clearing all routes');
        
        if (window.currentDijkstraRoute) {
            window.currentDijkstraRoute.setMap(null);
            window.currentDijkstraRoute = null;
        }
        
        if (window.currentAstarRoute) {
            window.currentAstarRoute.setMap(null);
            window.currentAstarRoute = null;
        }
    } catch (error) {
        console.error('Error clearing routes:', error);
    }
}

// Display route on map
function displayRouteOnMap(routeData, targetMap) {
    try {
        console.log('Displaying route on map:', targetMap);
        
        // Check if map is valid
        if (!targetMap || typeof targetMap.setCenter !== 'function') {
            console.error('Invalid map object:', targetMap);
            return;
        }
        
        // Check route data
        if (!routeData || !routeData.path || !Array.isArray(routeData.path) || routeData.path.length < 2) {
            console.error('Invalid route data:', routeData);
            return;
        }
        
        // Clear previous markers if any
        if (targetMap.markers) {
            targetMap.markers.forEach(marker => marker.setMap(null));
        }
        targetMap.markers = [];
        
        // Create path points for the final route
        const path = [];
        for (let i = 0; i < routeData.path.length; i++) {
            const city = routeData.path[i];
            if (!cityCoordinates[city]) {
                console.error('Missing coordinates for city:', city);
                continue;
            }
            path.push({
                lat: cityCoordinates[city].lat,
                lng: cityCoordinates[city].lng
            });
        }
        
        if (path.length < 2) {
            console.error('Not enough valid points in path');
            return;
        }
        
        // Draw the main route (red line)
        const route = new google.maps.Polyline({
            path: path,
            geodesic: true,
            strokeColor: '#FF0000',
            strokeOpacity: 1.0,
            strokeWeight: 3
        });
        
        route.setMap(targetMap);
        
        // Store reference to clear later
        if (targetMap === dijkstraMap) {
            window.currentDijkstraRoute = route;
        } else if (targetMap === astarMap) {
            window.currentAstarRoute = route;
        }
        
        // Show all visited nodes with markers if available
        if (routeData.visited_nodes && Array.isArray(routeData.visited_nodes)) {
            console.log(`Displaying ${routeData.visited_nodes.length} visited nodes`);
            
            // Create markers for visited nodes that are not part of the path
            for (let i = 0; i < routeData.visited_nodes.length; i++) {
                const city = routeData.visited_nodes[i];
                
                // Skip if this city is already in the path
                if (routeData.path.includes(city)) continue;
                
                if (!cityCoordinates[city]) {
                    console.error('Missing coordinates for visited node:', city);
                    continue;
                }
                
                const marker = new google.maps.Marker({
                    position: {
                        lat: cityCoordinates[city].lat,
                        lng: cityCoordinates[city].lng
                    },
                    map: targetMap,
                    title: `Visited: ${city}`,
                    icon: {
                        path: google.maps.SymbolPath.CIRCLE,
                        scale: 5,
                        fillColor: "#FFAA00",
                        fillOpacity: 1,
                        strokeWeight: 1,
                        strokeColor: "#FF6600"
                    }
                });
                
                // Store marker for later removal
                targetMap.markers.push(marker);
            }
        }
        
        // Add markers for the path cities with numbers
        for (let i = 0; i < routeData.path.length; i++) {
            const city = routeData.path[i];
            if (!cityCoordinates[city]) continue;
            
            // Create a numbered marker for path nodes
            const marker = new google.maps.Marker({
                position: {
                    lat: cityCoordinates[city].lat,
                    lng: cityCoordinates[city].lng
                },
                map: targetMap,
                title: city,
                label: {
                    text: (i + 1).toString(),
                    color: 'white'
                },
                zIndex: 1000 // Put path markers on top
            });
            
            // Store marker for later removal
            targetMap.markers.push(marker);
        }
        
        // Fit bounds to show the entire route plus visited nodes
        const bounds = new google.maps.LatLngBounds();
        
        // Include path points
        path.forEach(point => bounds.extend(point));
        
        // Include all visited nodes
        if (routeData.visited_nodes && Array.isArray(routeData.visited_nodes)) {
            for (let i = 0; i < routeData.visited_nodes.length; i++) {
                const city = routeData.visited_nodes[i];
                if (!cityCoordinates[city]) continue;
                
                bounds.extend({
                    lat: cityCoordinates[city].lat,
                    lng: cityCoordinates[city].lng
                });
            }
        }
        
        targetMap.fitBounds(bounds);
        
        // Add legend
        addLegend(targetMap, routeData.algorithm);
        
        console.log('Route displayed successfully');
    } catch (error) {
        console.error('Error displaying route on map:', error);
    }
}

// Add a legend to the map
function addLegend(map, algorithm) {
    // Remove existing legend if any
    const existingLegend = document.getElementById(`legend-${map === dijkstraMap ? 'dijkstra' : 'astar'}`);
    if (existingLegend) {
        existingLegend.remove();
    }
    
    // Create legend div
    const legend = document.createElement('div');
    legend.id = `legend-${map === dijkstraMap ? 'dijkstra' : 'astar'}`;
    legend.className = 'map-legend';
    
    // Style for the legend
    legend.style.backgroundColor = 'white';
    legend.style.padding = '10px';
    legend.style.margin = '10px';
    legend.style.border = '1px solid #ccc';
    legend.style.borderRadius = '5px';
    legend.style.boxShadow = '0 2px 4px rgba(0,0,0,0.1)';
    legend.style.fontSize = '12px';
    
    // Create content
    legend.innerHTML = `
        <h3 style="margin: 0 0 8px 0; font-size: 14px;">${algorithm === 'dijkstra' ? 'Dijkstra' : 'A*'} Legend</h3>
        <div style="display: flex; align-items: center; margin-bottom: 5px;">
            <div style="width: 20px; height: 3px; background-color: #FF0000; margin-right: 8px;"></div>
            <span>Final Path</span>
        </div>
        <div style="display: flex; align-items: center; margin-bottom: 5px;">
            <div style="width: 12px; height: 12px; border-radius: 50%; background-color: #FFAA00; margin-right: 8px; border: 1px solid #FF6600;"></div>
            <span>Visited Node</span>
        </div>
        <div style="display: flex; align-items: center;">
            <div style="width: 16px; height: 16px; background-color: #4285F4; border-radius: 50%; color: white; display: flex; align-items: center; justify-content: center; margin-right: 8px; font-size: 10px;">1</div>
            <span>Path Node (in order)</span>
        </div>
    `;
    
    // Position the legend at the top-right of the map
    map.controls[google.maps.ControlPosition.TOP_RIGHT].push(legend);
}

// Display route details
function displayRouteDetails(routeData) {
    try {
        const routeDetailsContainer = document.getElementById('route-details');
        if (!routeDetailsContainer) {
            console.error('Route details container not found');
            return;
        }
        
        if (!routeData) {
            console.error('No route data to display');
            routeDetailsContainer.innerHTML = '<p>No route data available</p>';
            return;
        }
        
        if (!routeData.path || !Array.isArray(routeData.path)) {
            console.error('Invalid path in route data:', routeData);
            routeDetailsContainer.innerHTML = '<p>Invalid route data</p>';
            return;
        }
        
        // Format details
        const distance = routeData.distance ? routeData.distance.toFixed(2) : 'N/A';
        const cost = routeData.cost ? routeData.cost.toFixed(2) : 'N/A';
        const time = routeData.time ? routeData.time.toFixed(2) : 'N/A';
        const stopsCount = routeData.stops ? routeData.stops.length : 0;
        const pathDetails = routeData.path.join(' → ');
        const optimization = routeData.optimization === 'time' ? 'Fastest Route' : 'Cheapest Route';
        
        routeDetailsContainer.innerHTML = `
            <h3>Route Details (${optimization})</h3>
            <div class="route-summary">
                <div class="route-stat">
                    <div class="stat-label">Distance</div>
                    <div class="stat-value">${distance} km</div>
                </div>
                <div class="route-stat">
                    <div class="stat-label">Cost</div>
                    <div class="stat-value">₹${cost}</div>
                </div>
                <div class="route-stat">
                    <div class="stat-label">Travel Time</div>
                    <div class="stat-value">${time} hours</div>
                </div>
                <div class="route-stat">
                    <div class="stat-label">Stops</div>
                    <div class="stat-value">${stopsCount}</div>
                </div>
                <div class="route-stat">
                    <div class="stat-label">Path</div>
                    <div class="stat-value path">${pathDetails}</div>
                </div>
            </div>
        `;
        
        // Add custom styles for route details
        const style = document.createElement('style');
        style.textContent = `
            .route-summary {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
                gap: 15px;
                margin-top: 15px;
            }
            .route-stat {
                background-color: #f5f5f5;
                border-radius: 8px;
                padding: 10px 15px;
                box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            }
            .stat-label {
                color: #666;
                font-size: 0.9rem;
                margin-bottom: 5px;
            }
            .stat-value {
                font-size: 1.1rem;
                font-weight: bold;
                color: #333;
            }
            .path {
                font-size: 0.9rem;
                word-break: break-word;
            }
        `;
        
        // Append style only once
        if (!document.getElementById('route-details-style')) {
            style.id = 'route-details-style';
            document.head.appendChild(style);
        }
    } catch (error) {
        console.error('Error displaying route details:', error);
    }
}

// Display algorithm comparison
function displayAlgorithmComparison(dijkstraData, astarData) {
    try {
        console.log('Updating comparison table with:', dijkstraData, astarData);
        
        if (!dijkstraData || !astarData) {
            console.error('Missing data for comparison table');
            return;
        }
        
        const dijkstraNodes = dijkstraData.nodes_visited || 
                             (dijkstraData.stats && dijkstraData.stats.nodes_visited) || 0;
        const dijkstraTime = dijkstraData.computation_time || 
                            (dijkstraData.stats && dijkstraData.stats.computation_time_ms) || 0;
        
        const astarNodes = astarData.nodes_visited || (astarData.stats && astarData.stats.nodes_visited) || 0;
        const astarTime = astarData.computation_time || 
                        (astarData.stats && astarData.stats.computation_time_ms) || 0;
        
        const dijkstraDistance = (dijkstraData.distance || dijkstraData.total_distance || 0);
        const dijkstraCost = (dijkstraData.cost || dijkstraData.total_cost || 0);
        const dijkstraTravelTime = (dijkstraData.time || dijkstraData.total_time || 0);
        
        const astarDistance = (astarData.distance || astarData.total_distance || 0);
        const astarCost = (astarData.cost || astarData.total_cost || 0);
        const astarTravelTime = (astarData.time || astarData.total_time || 0);
        
        // Format costs with Indian rupee symbol
        const formatCost = (cost) => `₹${cost.toFixed(2)}`;
        
        // Update comparison table cells
        document.getElementById('dijkstra-nodes').textContent = dijkstraNodes;
        document.getElementById('dijkstra-time').textContent = typeof dijkstraTime === 'number' ? 
                                                            dijkstraTime.toFixed(3) : dijkstraTime;
        document.getElementById('dijkstra-distance').textContent = dijkstraDistance.toFixed(2);
        document.getElementById('dijkstra-cost').textContent = formatCost(dijkstraCost);
        document.getElementById('dijkstra-travel-time').textContent = dijkstraTravelTime.toFixed(2);
        
        document.getElementById('astar-nodes').textContent = astarNodes;
        document.getElementById('astar-time').textContent = typeof astarTime === 'number' ? 
                                                          astarTime.toFixed(3) : astarTime;
        document.getElementById('astar-distance').textContent = astarDistance.toFixed(2);
        document.getElementById('astar-cost').textContent = formatCost(astarCost);
        document.getElementById('astar-travel-time').textContent = astarTravelTime.toFixed(2);
        
        // Add visual indicators to show which algorithm performed better
        highlightBetterAlgorithm('dijkstra-nodes', 'astar-nodes', false); // Lower is better
        highlightBetterAlgorithm('dijkstra-time', 'astar-time', false); // Lower is better
        highlightBetterAlgorithm('dijkstra-distance', 'astar-distance', false); // Lower is better
        highlightBetterAlgorithm('dijkstra-cost', 'astar-cost', false); // Lower is better
        highlightBetterAlgorithm('dijkstra-travel-time', 'astar-travel-time', false); // Lower is better
        
        console.log('Comparison table updated successfully');
    } catch (error) {
        console.error('Error updating comparison table:', error);
    }
}

// Helper function to highlight the better algorithm in comparison
function highlightBetterAlgorithm(dijkstraId, astarId, higherIsBetter = false) {
    const dijkstraElement = document.getElementById(dijkstraId);
    const astarElement = document.getElementById(astarId);
    
    if (!dijkstraElement || !astarElement) return;
    
    // Get the values as numbers
    const dijkstraValue = parseFloat(dijkstraElement.textContent);
    const astarValue = parseFloat(astarElement.textContent);
    
    // Reset styling
    dijkstraElement.style.fontWeight = 'normal';
    astarElement.style.fontWeight = 'normal';
    dijkstraElement.style.color = '';
    astarElement.style.color = '';
    
    // Return if values are equal or invalid
    if (isNaN(dijkstraValue) || isNaN(astarValue) || dijkstraValue === astarValue) return;
    
    // Determine which is better
    const dijkstraIsBetter = higherIsBetter ? (dijkstraValue > astarValue) : (dijkstraValue < astarValue);
    
    // Highlight the better one
    if (dijkstraIsBetter) {
        dijkstraElement.style.fontWeight = 'bold';
        dijkstraElement.style.color = '#009900'; // Green
    } else {
        astarElement.style.fontWeight = 'bold';
        astarElement.style.color = '#009900'; // Green
    }
} 