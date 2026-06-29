#pragma once

#include "core/Scene.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

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

    sf::Text m_titleText;
    sf::Text m_startText;
    sf::Text m_controlsText;
};
