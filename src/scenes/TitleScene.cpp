#include "scenes/TitleScene.hpp"
#include "core/Game.hpp"
#include "data/Constants.hpp"
#include "scenes/PlayScene.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace {
constexpr float VW = Config::VIEW_WIDTH;
constexpr float VH = Config::VIEW_HEIGHT;
int fs(float r) { return static_cast<int>(VH * r); }
} // namespace

TitleScene::TitleScene(Game& game)
    : m_game(game), m_titleText(*game.getFonts().get("default"), ""),
      m_startText(*game.getFonts().get("default"), ""),
      m_controlsText(*game.getFonts().get("default"), "") {
    auto fontPtr = m_game.getFonts().get("default");
    m_font = fontPtr.get();

    if (!m_font)
        return;

    m_titleText.setString("Survivor-like");
    m_titleText.setCharacterSize(fs(0.055f));
    m_titleText.setFillColor(sf::Color::White);
    m_titleText.setPosition({VW * 0.28f, VH * 0.15f});

    m_startText.setString("Press ENTER to start");
    m_startText.setCharacterSize(fs(0.025f));
    m_startText.setFillColor(sf::Color::Yellow);
    m_startText.setPosition({VW * 0.35f, VH * 0.28f});

    m_controlsText.setString("WASD to move  |  Weapons auto-fire  |  Escape to quit");
    m_controlsText.setCharacterSize(fs(0.015f));
    m_controlsText.setFillColor(sf::Color(160, 160, 160));
    m_controlsText.setPosition({VW * 0.19f, VH * 0.82f});
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
