#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cmath>

// Screen dimensions
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 800;

// Signal timing (seconds)
const int DEFAULT_RED = 15;
const int DEFAULT_YELLOW =5;
const int DEFAULT_GREEN = 10;

// Stop lines for each direction
const int STOP_LINES[] = { 590 - 60, 330 - 60, 800, 535 };


// Vehicle types and speeds
enum VehicleType { CAR, BUS, TRUCK, BIKE };
const float VEHICLE_SPEEDS[] = { 2.25f, 1.8f, 1.8f, 2.5f };

// Directions for vehicles
enum Direction { RIGHT, DOWN, LEFT, UP };
const sf::Vector2f signalCoords[] = {
    sf::Vector2f(530, 230), sf::Vector2f(810, 230),
    sf::Vector2f(810, 570), sf::Vector2f(530, 570)
};

// Traffic signal management
class TrafficSignal {
public:
    sf::Sprite sprite;
    int red, yellow, green, currentTime;
    bool isGreen, isYellow;

    TrafficSignal(int r, int y, int g)
        : red(r), yellow(y), green(g), currentTime(g), isGreen(false), isYellow(false) {}

    void update() {
        if (currentTime > 0) currentTime--;
    }
};

std::vector<TrafficSignal> signals;
int currentGreen = 0, nextGreen = 1;
bool currentYellow = false;

// Vehicle management
class Vehicle {
public:
    VehicleType type;
    sf::Sprite sprite;
    float speed;
    Direction direction;
    float stopPosition;
    bool stopped;
    bool crossedStopLine;
    bool willTurn; // Flag for whether the vehicle will turn
    bool turned;   // Flag for whether the turn is complete
    float rotateAngle; // Angle of rotation during a turn
    std::string identity;
    int vehicleIdCounter;

    Vehicle(const sf::Texture& texture, Direction dir, float spd, float stopPos, int id, VehicleType vType)
        : direction(dir), speed(spd), stopPosition(stopPos), stopped(false), crossedStopLine(false),
          willTurn(false), turned(false), rotateAngle(0.0f), type(vType) {
        sprite.setTexture(texture);
        identity = "ID-" + std::to_string(id);
        std::cout << identity << " vehicle spawned with speed: " << speed * 24.0f << " km/h\n";

        // Initial positions based on direction
        if (direction == RIGHT) sprite.setPosition(0, 375);
        else if (direction == DOWN) sprite.setPosition(708, 0);
        else if (direction == LEFT) sprite.setPosition(SCREEN_WIDTH, 430);
        else if (direction == UP) sprite.setPosition(602, SCREEN_HEIGHT);
    }

    void executeTurn() {
    if (direction == RIGHT) {
        // Rotate until 90 degrees
        rotateAngle += 3;
        sprite.setRotation(rotateAngle);
        if (rotateAngle >= 90) {
            sprite.setRotation(0);  // Reset rotation after turn
            direction = UP;         // Turn right -> UP
            willTurn = false;       // Stop the turn flag
        } else {
            sprite.move(2.4f, 0);  // Move slightly while rotating
        }
    } 
    else if (direction == DOWN) {
        // Rotate until 90 degrees
        rotateAngle += 3;
        sprite.setRotation(rotateAngle);
        if (rotateAngle >= 90) {
            sprite.setRotation(0);
            direction = RIGHT;      // Turn down -> RIGHT
            willTurn = false;       // Stop the turn flag
        } else {
            sprite.move(0, 2.8f);  // Move slightly while rotating
        }
    } 
    else if (direction == LEFT) {
        // Rotate until 90 degrees
        rotateAngle += 3;
        sprite.setRotation(rotateAngle);
        if (rotateAngle >= 90) {
            sprite.setRotation(0);
            direction = DOWN;       // Turn left -> DOWN
            willTurn = false;       // Stop the turn flag
        } else {
            sprite.move(-2.4f, 0); // Move slightly while rotating
        }
    } 
    else if (direction == UP) {
        // Rotate until 90 degrees
        rotateAngle += 3;
        sprite.setRotation(rotateAngle);
        if (rotateAngle >= 90) {
            sprite.setRotation(0);
            direction = LEFT;       // Turn up -> LEFT
            willTurn = false;       // Stop the turn flag
        } else {
            sprite.move(0, -2.8f); // Move slightly while rotating
        }
    }
}

void move() {
    if (!stopped) {
        if (willTurn) {
            // Handle turning logic
            executeTurn(); // Rotate vehicle
        }
        
        // After the turn, move according to the current direction
        if (!willTurn) { // Only move if no turn is happening
            if (direction == RIGHT) sprite.move(speed, 0); // Move right
            else if (direction == DOWN) sprite.move(0, speed); // Move down
            else if (direction == LEFT) sprite.move(-speed, 0); // Move left
            else if (direction == UP) sprite.move(0, -speed); // Move up
        }
    }
}





    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

// Traffic Simulation
class TrafficSimulation {
private:
    sf::Clock spawnClocks[4]; // Separate spawn clocks for each direction
    sf::Clock busClock; // Global clock for bus spawns

    sf::RenderWindow window;
    sf::Texture backgroundTexture, redSignalTexture, yellowSignalTexture, greenSignalTexture;
    sf::Sprite backgroundSprite;
    sf::Clock spawnClock;
    std::vector<Vehicle> vehicles;
     int vehicleIdCounter; // Unique vehicle ID counter
    // Directional Textures
    sf::Texture carTextures[4], busTextures[4], truckTextures[4], bikeTextures[4];

public:
    TrafficSimulation()
         : window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Traffic Simulation"), vehicleIdCounter(1) { // Start from 1
    window.setFramerateLimit(60);
        // Load textures
        backgroundTexture.loadFromFile("images/intersection.png");
        redSignalTexture.loadFromFile("images/signals/red.png");
        yellowSignalTexture.loadFromFile("images/signals/yellow.png");
        greenSignalTexture.loadFromFile("images/signals/green.png");

        // Load directional textures for each vehicle type
        carTextures[RIGHT].loadFromFile("images/right/car.png");
        carTextures[DOWN].loadFromFile("images/down/car.png");
        carTextures[LEFT].loadFromFile("images/left/car.png");
        carTextures[UP].loadFromFile("images/up/car.png");

        busTextures[RIGHT].loadFromFile("images/right/bus.png");
        busTextures[DOWN].loadFromFile("images/down/bus.png");
        busTextures[LEFT].loadFromFile("images/left/bus.png");
        busTextures[UP].loadFromFile("images/up/bus.png");

        truckTextures[RIGHT].loadFromFile("images/right/truck.png");
        truckTextures[DOWN].loadFromFile("images/down/truck.png");
        truckTextures[LEFT].loadFromFile("images/left/truck.png");
        truckTextures[UP].loadFromFile("images/up/truck.png");

        bikeTextures[RIGHT].loadFromFile("images/right/bike.png");
        bikeTextures[DOWN].loadFromFile("images/down/bike.png");
        bikeTextures[LEFT].loadFromFile("images/left/bike.png");
        bikeTextures[UP].loadFromFile("images/up/bike.png");

        // Background setup
        backgroundSprite.setTexture(backgroundTexture);

        // Initialize signals
        signals.emplace_back(DEFAULT_RED, DEFAULT_YELLOW, DEFAULT_GREEN);
        signals.emplace_back(DEFAULT_RED, DEFAULT_YELLOW, DEFAULT_GREEN);
        signals.emplace_back(DEFAULT_RED, DEFAULT_YELLOW, DEFAULT_GREEN);
        signals.emplace_back(DEFAULT_RED, DEFAULT_YELLOW, DEFAULT_GREEN);
    }


    void updateSignals() {
        while (true) {
            // Green phase
            signals[currentGreen].isGreen = true;
            signals[currentGreen].isYellow = false;
            signals[currentGreen].currentTime = DEFAULT_GREEN;

            for (int i = 0; i < DEFAULT_GREEN; i++) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            // Yellow phase
            signals[currentGreen].isGreen = false;
            signals[currentGreen].isYellow = true;
            signals[currentGreen].currentTime = DEFAULT_YELLOW;

            for (int i = 0; i < DEFAULT_YELLOW; i++) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            signals[currentGreen].isYellow = false;

            // Move to next signal
            currentGreen = nextGreen;
            nextGreen = (currentGreen + 1) % signals.size();
        }
    }

    float getRandomSpeed(VehicleType type) {
    switch (type) {
        case CAR:
        case BIKE:
            return 1.5f + static_cast<float>(std::rand()) / RAND_MAX * (2.5f - 1.5f); // Random speed 1.5-2.5 km/h
        case TRUCK:
            return 3.5f; // Constant speed 3.5 km/h
        case BUS:
            return 0.5f + static_cast<float>(std::rand()) / RAND_MAX * (1.5f - 0.5f); // Random speed 0.5-1.5 km/h
        default:
            return 2.0f; // Default speed in case of error
    }
}

void manageVehicleStops() {
    for (auto& vehicle : vehicles) {
        bool isSignalGreen = signals[vehicle.direction].isGreen;
        bool isSignalYellow = signals[vehicle.direction].isYellow;
        bool isSignalRed = !isSignalGreen && !isSignalYellow;

        // Trucks (emergency vehicles) ignore signals and stop lines
        if (vehicle.type == TRUCK) {
            vehicle.stopped = false;
            vehicle.crossedStopLine = true; // Trucks bypass the stop line logic
            std::cout << "Truck ID " << vehicle.identity << " bypassing signal." << std::endl;
            continue;
        }

        // Update crossedStopLine flag for other vehicles
        if (!vehicle.crossedStopLine) {
            if (vehicle.direction == RIGHT && vehicle.sprite.getPosition().x >= vehicle.stopPosition) {
                vehicle.crossedStopLine = true;
            } else if (vehicle.direction == DOWN && vehicle.sprite.getPosition().y >= vehicle.stopPosition) {
                vehicle.crossedStopLine = true;
            } else if (vehicle.direction == LEFT && vehicle.sprite.getPosition().x <= vehicle.stopPosition) {
                vehicle.crossedStopLine = true;
            } else if (vehicle.direction == UP && vehicle.sprite.getPosition().y <= vehicle.stopPosition) {
                vehicle.crossedStopLine = true;
            }
        }

        // If the vehicle has not crossed the stop line, they must stop if the signal is red
        if (!vehicle.crossedStopLine) {
            if ((vehicle.direction == RIGHT && vehicle.sprite.getPosition().x + vehicle.speed >= vehicle.stopPosition) ||
                (vehicle.direction == DOWN && vehicle.sprite.getPosition().y + vehicle.speed >= vehicle.stopPosition) ||
                (vehicle.direction == LEFT && vehicle.sprite.getPosition().x - vehicle.speed <= vehicle.stopPosition) ||
                (vehicle.direction == UP && vehicle.sprite.getPosition().y - vehicle.speed <= vehicle.stopPosition)) {
                vehicle.stopped = isSignalRed; // Stop only if the signal is red
            } else {
                vehicle.stopped = false; // Allow movement if the signal is green
            }
        } else {
            // Once crossed stop line, check if the vehicle is allowed to turn
            if (isSignalGreen && !vehicle.turned && !vehicle.willTurn) {
                vehicle.willTurn = true; // Allow turning if signal is green and the vehicle has crossed the stop line
            }

            vehicle.stopped = false; // Vehicles that crossed the stop line keep moving
        }
    }
}


void spawnVehicle() {
    // Handle bus spawning globally every 15 seconds
    if (busClock.getElapsedTime().asSeconds() >= 15.0f) {
        for (int dir = 0; dir < 4; ++dir) {
            float speed = getRandomSpeed(BUS);
            
            vehicles.emplace_back(
                busTextures[dir],              // Bus texture for the current direction
                static_cast<Direction>(dir),   // Direction
                speed,                         // Speed
                STOP_LINES[dir],               // Stop position
                vehicleIdCounter++,            // Unique ID
                BUS                            // Vehicle type
            );

            std::cout << "Bus spawned at direction " << dir << " with speed: " << speed << "km/h, ID: " << vehicleIdCounter - 1 << std::endl;
        }
        busClock.restart();
    }

    // Spawn vehicles with random intervals
    for (int dir = 0; dir < 4; ++dir) {
        float spawnInterval = 2.0f; // Base interval for car/bike spawn

        // Spawning cars and bikes with random intervals
        if (spawnClocks[dir].getElapsedTime().asSeconds() >= spawnInterval) {
            VehicleType type = (std::rand() % 2 == 0) ? CAR : BIKE;
            float speed = getRandomSpeed(type);
            bool willTurn = (std::rand() % 100 < 30); // 30% chance to turn

            Vehicle vehicle(
                type == CAR ? carTextures[dir] : bikeTextures[dir],
                static_cast<Direction>(dir),
                speed,
                STOP_LINES[dir],
                vehicleIdCounter++,
                type
            );

            // Set willTurn flag only after reaching stop line and if the signal is green
            vehicle.willTurn = false;

            vehicles.push_back(vehicle);

            std::cout << vehicle.identity << " spawned at direction " << dir << (vehicle.willTurn ? " (will turn)" : "") << "\n";
            spawnClocks[dir].restart();
        }

        // Spawning trucks with specific probability and interval
        float truckSpawnInterval = 0.0f;
        float truckProbability = 0.0f;

        switch (dir) {
            case DOWN:
                truckSpawnInterval = 15.0f;   // Truck spawn interval
                truckProbability = 0.2f;      // Truck spawn probability
                break;
            case UP:
                truckSpawnInterval = 2.0f;    // Truck spawn interval
                truckProbability = 0.05f;     // Truck spawn probability
                break;
            case LEFT:
                truckSpawnInterval = 20.0f;   // Truck spawn interval
                truckProbability = 0.1f;      // Truck spawn probability
                break;
            case RIGHT:
                truckSpawnInterval = 2.0f;    // Truck spawn interval
                truckProbability = 0.3f;      // Truck spawn probability
                break;
        }

        if (spawnClocks[dir].getElapsedTime().asSeconds() >= truckSpawnInterval && static_cast<float>(std::rand()) / RAND_MAX <= truckProbability) {
            float speed = getRandomSpeed(TRUCK);
            vehicles.emplace_back(
                truckTextures[dir],                  // Truck texture
                static_cast<Direction>(dir),         // Direction
                speed,                               // Speed
                STOP_LINES[dir],                     // Stop position
                vehicleIdCounter++,                  // Unique ID
                TRUCK                                // Vehicle type
            );

            std::cout << "Truck spawned at direction " << dir << " with speed: " << speed << "km/h, ID: " << vehicleIdCounter - 1 << std::endl;
            spawnClocks[dir].restart();
        }
    }
}








    void run() {
    std::thread signalThread(&TrafficSimulation::updateSignals, this);
    signalThread.detach();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        spawnVehicle();
        manageVehicleStops();

        // Render the simulation
        window.clear();
        window.draw(backgroundSprite);

        for (int i = 0; i < signals.size(); i++) {
            sf::Sprite signalSprite;
            if (signals[i].isGreen) {
                signalSprite.setTexture(greenSignalTexture);
            } else if (signals[i].isYellow) {
                signalSprite.setTexture(yellowSignalTexture);
            } else {
                signalSprite.setTexture(redSignalTexture);
            }
            signalSprite.setPosition(signalCoords[i]);
            window.draw(signalSprite);
        }

        for (auto& vehicle : vehicles) {
            vehicle.move();
            vehicle.draw(window);
        }

        window.display();
    }
}

};

int main() {
    TrafficSimulation simulation;
    simulation.run();
    return 0;
}
