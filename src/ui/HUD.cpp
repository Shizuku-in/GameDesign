#include "ui/HUD.hpp"
#include "data/Constants.hpp"
#include "gameplay/WeaponDefs.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdio>
#include <string>

namespace {
constexpr float VW = Config::VIEW_WIDTH;
constexpr float VH = Config::VIEW_HEIGHT;
int fs(float ratio) { return static_cast<int>(VH * ratio); }
} // namespace

HUD::HUD(const sf::Font& font)
    : m_font(font), m_hpLabel(font, "", fs(0.015f)), m_xpLabel(font, "", fs(0.013f)),
      m_levelText(font, "", fs(0.022f)), m_timerText(font, "", fs(0.017f)),
      m_weaponList(font, "", fs(0.014f)) {
    // HP 条
    float hpW = VW * 0.25f;
    float hpH = VH * 0.017f;
    m_hpBarBg.setSize({hpW, hpH});
    m_hpBarBg.setPosition({VW * 0.01f, VH * 0.01f});
    m_hpBarBg.setFillColor(Config::COLOR_UI_HP_BG);

    m_hpBarFill.setSize({hpW, hpH});
    m_hpBarFill.setPosition({VW * 0.01f, VH * 0.01f});
    m_hpBarFill.setFillColor(sf::Color::Red);

    m_hpLabel.setFillColor(sf::Color::White);
    m_hpLabel.setPosition({VW * 0.012f, VH * 0.01f});

    // XP 条
    float xpW = VW * 0.98f;
    float xpH = VH * 0.013f;
    m_xpBarBg.setSize({xpW, xpH});
    m_xpBarBg.setPosition({VW * 0.01f, VH * 0.96f});
    m_xpBarBg.setFillColor(Config::COLOR_UI_XP_BG);

    m_xpBarFill.setSize({0.f, xpH});
    m_xpBarFill.setPosition({VW * 0.01f, VH * 0.96f});
    m_xpBarFill.setFillColor(sf::Color::Yellow);

    m_xpLabel.setFillColor(sf::Color::White);
    m_xpLabel.setPosition({VW * 0.375f, VH * 0.955f});

    // 等级
    m_levelText.setFillColor(sf::Color::White);
    m_levelText.setPosition({VW * 0.45f, VH * 0.005f});

    // 计时器
    m_timerText.setFillColor(sf::Color::White);
    m_timerText.setPosition({VW * 0.875f, VH * 0.005f});

    // 武器列表
    m_weaponList.setFillColor(Config::COLOR_TEXT_DEFAULT);
    m_weaponList.setPosition({VW * 0.775f, VH * 0.04f});
}

void HUD::update(const PlayerState& player, const WeaponSystem& weapons, float gameTime) {
    char buf[64];

    // HP
    float hpFrac = player.hp / player.maxHp;
    if (hpFrac < 0.f)
        hpFrac = 0.f;
    m_hpBarFill.setSize({VW * 0.25f * hpFrac, VH * 0.017f});
    std::snprintf(buf, sizeof(buf), "HP %d/%d", static_cast<int>(player.hp),
                  static_cast<int>(player.maxHp));
    m_hpLabel.setString(buf);

    // XP
    float xpFrac = player.xp / player.xpToNext;
    if (xpFrac > 1.f)
        xpFrac = 1.f;
    m_xpBarFill.setSize({VW * 0.98f * xpFrac, VH * 0.013f});
    std::snprintf(buf, sizeof(buf), "XP %d/%d", static_cast<int>(player.xp),
                  static_cast<int>(player.xpToNext));
    m_xpLabel.setString(buf);

    // 等级
    std::snprintf(buf, sizeof(buf), "Lv.%d", player.level);
    m_levelText.setString(buf);

    // 计时器（分:秒）
    int totalSec = static_cast<int>(gameTime);
    std::snprintf(buf, sizeof(buf), "%02d:%02d", totalSec / 60, totalSec % 60);
    m_timerText.setString(buf);

    // 武器（多行）
    std::string weaponStr;
    for (int i = 0; i < static_cast<int>(WeaponType::Count); ++i) {
        auto wt = static_cast<WeaponType>(i);
        int lvl = weapons.getLevel(wt);
        if (lvl > 0) {
            const auto& def = WEAPON_DEFS[i];
            std::snprintf(buf, sizeof(buf), "%s Lv.%d\n", def.name, lvl);
            weaponStr += buf;
        }
    }
    m_weaponList.setString(weaponStr);
}

void HUD::render(sf::RenderWindow& window) {
    window.draw(m_hpBarBg);
    window.draw(m_hpBarFill);
    window.draw(m_hpLabel);
    window.draw(m_xpBarBg);
    window.draw(m_xpBarFill);
    window.draw(m_xpLabel);
    window.draw(m_levelText);
    window.draw(m_timerText);
    window.draw(m_weaponList);
}
