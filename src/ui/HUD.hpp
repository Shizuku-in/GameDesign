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
    /// 使用指定字体创建固定屏幕空间的 HUD。
    explicit HUD(const sf::Font& font);

    /// 使用当前玩家、武器与游戏时间刷新常规 HUD 内容。
    void update(const PlayerState& player, const WeaponSystem& weapons, float gameTime);
    /// 更新单独按渲染帧率采样的 FPS 文本。
    void setFramesPerSecond(float framesPerSecond);
    /// 在默认视图中绘制所有 HUD 控件。
    void render(sf::RenderWindow& window);

private:
    /// HUD 使用的共享字体。
    const sf::Font& m_font;

    /// 生命条背景。
    sf::RectangleShape m_hpBarBg;
    /// 按当前生命比例缩放的生命条填充。
    sf::RectangleShape m_hpBarFill;
    /// 当前与最大生命值文本。
    sf::Text m_hpLabel;

    /// 经验条背景。
    sf::RectangleShape m_xpBarBg;
    /// 按当前经验比例缩放的经验条填充。
    sf::RectangleShape m_xpBarFill;
    /// 当前与升级所需经验文本。
    sf::Text m_xpLabel;

    /// 玩家等级文本。
    sf::Text m_levelText;
    /// 本局累计时间文本。
    sf::Text m_timerText;
    /// 实时渲染帧率文本。
    sf::Text m_fpsText;

    /// 已装备武器及等级的多行文本块。
    sf::Text m_weaponList;
};
