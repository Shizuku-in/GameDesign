#pragma once

#include "Scene.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

class Game;

class GameScene : public Scene {
public:
    explicit GameScene(Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time dt) override;
    void render(sf::RenderWindow& window) override;

private:
    void checkBounds(const sf::Vector2u& windowSize);

    Game& m_game;
    sf::CircleShape m_player;
    sf::Vector2f m_velocity;

    // Background color cycling
    sf::Color m_bgColor;
    float m_bgHue = 0.f; // hue in [0, 360)

    static constexpr float PLAYER_RADIUS = 40.f;
    static constexpr float PLAYER_SPEED = 300.f; // pixels per second
    static constexpr float BG_HUE_SPEED = 30.f;  // degrees per second
};
