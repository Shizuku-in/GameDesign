#include "scenes/TitleScene.hpp"

#include "core/Game.hpp"
#include "data/Constants.hpp"
#include "scenes/PlayScene.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <cmath>
#include <cstdio>

namespace {
constexpr float VW = Config::VIEW_WIDTH;
constexpr float VH = Config::VIEW_HEIGHT;
int fs(float r) { return static_cast<int>(VH * r); }
} // namespace

TitleScene::TitleScene(Game& game)
    : m_game(game), m_startText(*game.getFonts().get("default"), ""),
      m_controlsText(*game.getFonts().get("default"), "") {
    auto fontPtr = m_game.getFonts().get("default");
    m_font = fontPtr.get();

    if (m_font == nullptr) {
        return;
    }

    // Logo
    if (!m_logoTexture.loadFromFile(Config::LOGO_PATH)) {
        std::fprintf(stderr, "[WARN] TitleScene: failed to load logo: %s\n", Config::LOGO_PATH);
    } else {
        m_logoTexture.setSmooth(false); // 像素艺术保持锐利
        m_logoSprite.emplace(m_logoTexture);

        // 缩放到合适宽度（约 550px）并保持比例
        float logoScale = 550.f / static_cast<float>(m_logoTexture.getSize().x);
        m_logoSprite->setScale({logoScale, logoScale});

        // 居中
        auto bounds = m_logoSprite->getGlobalBounds();
        m_logoSprite->setPosition({(VW - bounds.size.x) / 2.f, VH * 0.10f});
    }

    // "按 ENTER 开始" — 呼吸闪烁
    m_startText.setString("Press ENTER to start");
    m_startText.setCharacterSize(fs(0.025f));
    m_startText.setFillColor(sf::Color::Yellow);
    {
        auto tb = m_startText.getLocalBounds();
        m_startText.setPosition({(VW - tb.size.x) / 2.f, VH * 0.52f});
    }

    // 操作提示
    m_controlsText.setString("WASD to move  |  Weapons auto-fire  |  Escape to quit");
    m_controlsText.setCharacterSize(fs(0.015f));
    m_controlsText.setFillColor(Config::COLOR_TEXT_MUTED);
    {
        auto tb = m_controlsText.getLocalBounds();
        m_controlsText.setPosition({(VW - tb.size.x) / 2.f, VH * 0.88f});
    }
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

void TitleScene::update(sf::Time dt) { m_elapsed += dt.asSeconds(); }

void TitleScene::render(sf::RenderWindow& window) {
    window.clear(Config::COLOR_BG_TITLE);

    if (m_font == nullptr) {
        return;
    }

    if (m_logoSprite) {
        window.draw(*m_logoSprite);
    }

    // ENTER 文字呼吸闪烁：alpha 在 155–255 间以 3 rad/s 振荡
    float alpha = 155.f + 100.f * (std::sin(m_elapsed * 3.f) * 0.5f + 0.5f);
    m_startText.setFillColor(
        sf::Color(255, 255, 0, static_cast<std::uint8_t>(std::clamp(alpha, 0.f, 255.f))));
    window.draw(m_startText);

    window.draw(m_controlsText);
}
