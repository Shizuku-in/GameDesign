#include "systems/HUD.hpp"
#include "systems/WeaponDefs.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdio>
#include <string>

HUD::HUD(const sf::Font& font)
    : m_font(font), m_hpLabel(font, "", 14), m_xpLabel(font, "", 12), m_levelText(font, "", 20),
      m_timerText(font, "", 16), m_weaponList(font, "", 13) {
    // HP bar
    m_hpBarBg.setSize({200.f, 16.f});
    m_hpBarBg.setPosition({10.f, 10.f});
    m_hpBarBg.setFillColor(sf::Color(60, 0, 0));

    m_hpBarFill.setSize({200.f, 16.f});
    m_hpBarFill.setPosition({10.f, 10.f});
    m_hpBarFill.setFillColor(sf::Color::Red);

    m_hpLabel.setFillColor(sf::Color::White);
    m_hpLabel.setPosition({12.f, 10.f});

    // XP bar
    m_xpBarBg.setSize({780.f, 12.f});
    m_xpBarBg.setPosition({10.f, 578.f});
    m_xpBarBg.setFillColor(sf::Color(60, 60, 0));

    m_xpBarFill.setSize({0.f, 12.f});
    m_xpBarFill.setPosition({10.f, 578.f});
    m_xpBarFill.setFillColor(sf::Color::Yellow);

    m_xpLabel.setFillColor(sf::Color::White);
    m_xpLabel.setPosition({300.f, 576.f});

    // Level
    m_levelText.setFillColor(sf::Color::White);
    m_levelText.setPosition({360.f, 6.f});

    // Timer
    m_timerText.setFillColor(sf::Color::White);
    m_timerText.setPosition({700.f, 8.f});

    // Weapon list
    m_weaponList.setFillColor(sf::Color(200, 200, 200));
    m_weaponList.setPosition({620.f, 40.f});
}

void HUD::update(const PlayerState& player, const WeaponSystem& weapons, float gameTime) {
    char buf[64];

    // HP
    float hpFrac = player.hp / player.maxHp;
    if (hpFrac < 0.f)
        hpFrac = 0.f;
    m_hpBarFill.setSize({200.f * hpFrac, 16.f});
    std::snprintf(buf, sizeof(buf), "HP %d/%d", static_cast<int>(player.hp),
                  static_cast<int>(player.maxHp));
    m_hpLabel.setString(buf);

    // XP
    float xpFrac = player.xp / player.xpToNext;
    if (xpFrac > 1.f)
        xpFrac = 1.f;
    m_xpBarFill.setSize({780.f * xpFrac, 12.f});
    std::snprintf(buf, sizeof(buf), "XP %d/%d", static_cast<int>(player.xp),
                  static_cast<int>(player.xpToNext));
    m_xpLabel.setString(buf);

    // Level
    std::snprintf(buf, sizeof(buf), "Lv.%d", player.level);
    m_levelText.setString(buf);

    // Timer (MM:SS)
    int totalSec = static_cast<int>(gameTime);
    std::snprintf(buf, sizeof(buf), "%02d:%02d", totalSec / 60, totalSec % 60);
    m_timerText.setString(buf);

    // Weapons (multi-line)
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
