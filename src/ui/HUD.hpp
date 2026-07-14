#pragma once

#include "data/PlayerState.hpp"
#include "systems/WeaponSystem.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

/// 在屏幕空间绘制 HP 条、XP 条、等级、计时器、帧数和武器列表。
class HUD {
public:
    explicit HUD(const sf::Font& font);

    void update(const PlayerState& player, const WeaponSystem& weapons, float gameTime);
    void setFramesPerSecond(float framesPerSecond);
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
    sf::Text m_fpsText;

    // 所有武器显示为单个多行文本块
    sf::Text m_weaponList;
};
