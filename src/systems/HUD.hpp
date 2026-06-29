#pragma once

#include "data/PlayerState.hpp"
#include "systems/WeaponSystem.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

/// Renders HP bar, XP bar, level, timer, and weapon list in screen space.
class HUD {
public:
    explicit HUD(const sf::Font& font);

    void update(const PlayerState& player, const WeaponSystem& weapons, float gameTime);
    void render(sf::RenderWindow& window);

private:
    const sf::Font& m_font;

    sf::RectangleShape m_hpBarBg;
    sf::RectangleShape m_hpBarFill;
    sf::Text m_hpLabel;

    sf::RectangleShape m_xpBarBg;
    sf::RectangleShape m_xpBarFill;
    sf::Text m_xpLabel;

    sf::Text m_levelText;
    sf::Text m_timerText;

    // All weapons displayed as a single multi-line text block
    sf::Text m_weaponList;
};
