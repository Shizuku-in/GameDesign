#include "scenes/GameOverScene.hpp"
#include "core/Game.hpp"
#include "scenes/PlayScene.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <cstdio>

GameOverScene::GameOverScene(Game& game, int score, int level, float survivalTime)
    : m_game(game), m_titleText(*game.getFonts().get("default"), ""),
      m_scoreText(*game.getFonts().get("default"), ""),
      m_levelText(*game.getFonts().get("default"), ""),
      m_timeText(*game.getFonts().get("default"), ""),
      m_restartText(*game.getFonts().get("default"), ""), m_score(score), m_level(level),
      m_survivalTime(survivalTime) {
    auto fontPtr = m_game.getFonts().get("default");
    m_font = fontPtr.get();

    if (!m_font)
        return;

    m_titleText.setString("GAME OVER");
    m_titleText.setCharacterSize(48);
    m_titleText.setFillColor(sf::Color::Red);
    m_titleText.setPosition({260.f, 120.f});

    char buf[128];

    std::snprintf(buf, sizeof(buf), "Enemies defeated: %d", m_score);
    m_scoreText.setString(buf);
    m_scoreText.setCharacterSize(24);
    m_scoreText.setFillColor(sf::Color::White);
    m_scoreText.setPosition({260.f, 220.f});

    std::snprintf(buf, sizeof(buf), "Level reached: %d", m_level);
    m_levelText.setString(buf);
    m_levelText.setCharacterSize(24);
    m_levelText.setFillColor(sf::Color::White);
    m_levelText.setPosition({280.f, 270.f});

    int totalSec = static_cast<int>(m_survivalTime);
    int min = totalSec / 60;
    int sec = totalSec % 60;
    std::snprintf(buf, sizeof(buf), "Survived: %02d:%02d", min, sec);
    m_timeText.setString(buf);
    m_timeText.setCharacterSize(24);
    m_timeText.setFillColor(sf::Color::White);
    m_timeText.setPosition({300.f, 320.f});

    m_restartText.setString("Press ENTER to play again");
    m_restartText.setCharacterSize(20);
    m_restartText.setFillColor(sf::Color::Yellow);
    m_restartText.setPosition({255.f, 440.f});
}

void GameOverScene::handleEvent(const sf::Event& event) {
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        using Key = sf::Keyboard::Key;
        if (kp->code == Key::Enter || kp->code == Key::Space) {
            m_game.changeScene(std::make_unique<PlayScene>(m_game));
        }
        if (kp->code == Key::Escape) {
            m_game.getWindow().close();
        }
    }
}

void GameOverScene::update(sf::Time /*dt*/) {}

void GameOverScene::render(sf::RenderWindow& window) {
    window.clear(sf::Color(20, 0, 0));

    if (!m_font)
        return;

    window.draw(m_titleText);
    window.draw(m_scoreText);
    window.draw(m_levelText);
    window.draw(m_timeText);
    window.draw(m_restartText);
}
