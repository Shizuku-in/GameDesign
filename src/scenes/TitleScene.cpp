#include "scenes/TitleScene.hpp"
#include "core/Game.hpp"
#include "scenes/PlayScene.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

TitleScene::TitleScene(Game& game)
    : m_game(game), m_titleText(*game.getFonts().get("default"), ""),
      m_startText(*game.getFonts().get("default"), ""),
      m_controlsText(*game.getFonts().get("default"), "") {
    auto fontPtr = m_game.getFonts().get("default");
    m_font = fontPtr.get();

    if (!m_font)
        return;

    m_titleText.setString("Survivor-like");
    m_titleText.setCharacterSize(48);
    m_titleText.setFillColor(sf::Color::White);
    m_titleText.setPosition({220.f, 180.f});

    m_startText.setString("Press ENTER to start");
    m_startText.setCharacterSize(22);
    m_startText.setFillColor(sf::Color::Yellow);
    m_startText.setPosition({280.f, 300.f});

    m_controlsText.setString("WASD to move  |  Weapons auto-fire  |  Escape to quit");
    m_controlsText.setCharacterSize(14);
    m_controlsText.setFillColor(sf::Color(160, 160, 160));
    m_controlsText.setPosition({150.f, 450.f});
}

void TitleScene::handleEvent(const sf::Event& event) {
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

void TitleScene::update(sf::Time /*dt*/) {}

void TitleScene::render(sf::RenderWindow& window) {
    window.clear(sf::Color(20, 20, 20));

    if (!m_font)
        return;

    window.draw(m_titleText);
    window.draw(m_startText);
    window.draw(m_controlsText);
}
