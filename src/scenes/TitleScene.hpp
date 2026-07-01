#pragma once

#include "core/Scene.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <optional>

class Game;

class TitleScene : public Scene {
public:
    explicit TitleScene(Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time dt) override;
    void render(sf::RenderWindow& window) override;

private:
    Game& m_game;
    const sf::Font* m_font = nullptr;

    sf::Texture m_logoTexture;
    std::optional<sf::Sprite> m_logoSprite;
    sf::Text m_startText;
    sf::Text m_controlsText;

    float m_elapsed = 0.f; // 用于呼吸灯动画
};
