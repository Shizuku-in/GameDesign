#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

namespace sf {
class RenderWindow;
}

class Scene {
public:
    virtual ~Scene() = default;

    virtual void handleEvent(const sf::Event&) {}
    virtual void update(sf::Time dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};
