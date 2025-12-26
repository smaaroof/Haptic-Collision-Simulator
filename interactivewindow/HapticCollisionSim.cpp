#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput9_1_0.lib")
#include <vector>
#include <cmath>
#include <algorithm>

class HapticController {
private:
    int controllerId;

public:
    HapticController(int id = 0) : controllerId(id) {}

    bool isConnected() {
        XINPUT_STATE state;
        return (XInputGetState(controllerId, &state) == ERROR_SUCCESS);
    }

    void setVibration(float leftMotor, float rightMotor) {
        XINPUT_VIBRATION vibration;
        vibration.wLeftMotorSpeed = static_cast<WORD>(leftMotor * 65535.f);
        vibration.wRightMotorSpeed = static_cast<WORD>(rightMotor * 65535.f);
        XInputSetState(controllerId, &vibration);
    }

    void stopVibration() {
        setVibration(0.f, 0.f);
    }
};

bool checkCollision(const sf::CircleShape& circle, const sf::RectangleShape& rect) {
    // get position and size function
    sf::Vector2f circlePos = circle.getPosition();
    float circleRadius = circle.getRadius();
    sf::Vector2f rectPos = rect.getPosition();
    sf::Vector2f rectSize = rect.getSize();

    // calculate circle center 
    sf::Vector2f circleCenter = circlePos + sf::Vector2f(circleRadius, circleRadius);

    // find the closest point on the rectangle to the circles center
    float closestX = std::max(rectPos.x, std::min(circleCenter.x, rectPos.x + rectSize.x));
    float closestY = std::max(rectPos.y, std::min(circleCenter.y, rectPos.y + rectSize.y));

    // calculate distance between the circles center and the closest point
    float distanceX = circleCenter.x - closestX;
    float distanceY = circleCenter.y - closestY;

    // distance less than radius = collision
    return (distanceX * distanceX + distanceY * distanceY) <= (circleRadius * circleRadius);
}

// obstacle class
struct Obstacle {
    sf::RectangleShape shape;
    float vibrationIntensity;
    bool isBouncy;
    std::string type;

    Obstacle(sf::Vector2f size, sf::Vector2f position, sf::Color color,
        float vibrationIntensity, bool isBouncy, std::string type)
        : vibrationIntensity(vibrationIntensity), isBouncy(isBouncy), type(type) {
        shape.setSize(size);
        shape.setPosition(position);
        shape.setFillColor(color);
    }
};

int main()
{
    // create window
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Haptic Environment");
    window.setFramerateLimit(60);

    // create circle cursor
    sf::CircleShape cursor(15.f);
    cursor.setFillColor(sf::Color::Blue);
    cursor.setPosition({ 400.f, 300.f });
    float cursorSpeed = 5.0f;

    // create different types of obstacles
    std::vector<Obstacle> obstacles;

    // solid wall (strong constant vibration)
    obstacles.push_back(Obstacle({ 200.f, 50.f }, { 300.f, 200.f },
        sf::Color::Red, 0.7f, false, "solid"));

    // bouncy obstacle (pulse feedback)
    obstacles.push_back(Obstacle({ 80.f, 80.f }, { 500.f, 400.f },
        sf::Color::Green, 0.5f, true, "bouncy"));

    // weak obstacle (subtle vibration)
    obstacles.push_back(Obstacle({ 120.f, 30.f }, { 200.f, 500.f },
        sf::Color::Yellow, 0.3f, false, "weak"));

    // dangerous obstacle (strong pulse vibration)
    obstacles.push_back(Obstacle({ 60.f, 60.f }, { 600.f, 100.f },
        sf::Color::Magenta, 0.9f, false, "dangerous"));

    // create haptic controller
    HapticController controller;

    // game loop
    while (window.isOpen())
    {
        for (auto event = window.pollEvent(); event.has_value(); event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        sf::Vector2f previousPosition = cursor.getPosition();

        // keyboard input handler
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            cursor.move({ -cursorSpeed, 0 });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            cursor.move({ cursorSpeed, 0 });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            cursor.move({ 0, -cursorSpeed });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            cursor.move({ 0, cursorSpeed });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            window.close();
        }

        // collision check with all obstacles
        bool isColliding = false;

        for (auto& obstacle : obstacles) {
            if (checkCollision(cursor, obstacle.shape)) {
                isColliding = true;

                // apply feedback based on the obstacle type
                if (controller.isConnected()) {
                    controller.setVibration(obstacle.vibrationIntensity, obstacle.vibrationIntensity);
                }

                cursor.setPosition(previousPosition);
                break;
            }
        }

        // stop vibration if not colliding
        if (!isColliding && controller.isConnected()) {
            controller.stopVibration();
        }

        window.clear(sf::Color::Black);

        // draw all obstacles
        for (const auto& obstacle : obstacles) {
            window.draw(obstacle.shape);
        }

        window.draw(cursor);
        window.display();
    }

    controller.stopVibration();
    return 0;
}