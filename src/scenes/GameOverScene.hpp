#pragma once

#include "core/Scene.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

class Game;

class GameOverScene : public Scene {
public:
    GameOverScene(Game& game, int score, int level, float survivalTime);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time dt) override;
    void render(sf::RenderWindow& window) override;

private:
    Game& m_game;
    const sf::Font* m_font = nullptr;

    sf::Text m_titleText;
    sf::Text m_scoreText;
    sf::Text m_levelText;
    sf::Text m_timeText;
    sf::Text m_restartText;

    int m_score;
    int m_level;
    float m_survivalTime;
};
