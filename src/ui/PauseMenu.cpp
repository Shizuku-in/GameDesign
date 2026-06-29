#include "ui/PauseMenu.hpp"
#include "data/Constants.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace {
constexpr float VW = Config::VIEW_WIDTH;
constexpr float VH = Config::VIEW_HEIGHT;
int fs(float r) { return static_cast<int>(VH * r); }
} // namespace

namespace PauseMenu {

void draw(sf::RenderWindow& window, const sf::Font& font, int selected) {
    // 半透明遮罩
    sf::RectangleShape overlay({VW, VH});
    overlay.setPosition({0.f, 0.f});
    overlay.setFillColor(Config::COLOR_OVERLAY_PAUSE);
    window.draw(overlay);

    // 标题
    sf::Text title(font);
    title.setString("PAUSED");
    title.setCharacterSize(fs(0.04f));
    title.setFillColor(sf::Color::White);
    title.setPosition({VW * 0.39f, VH * 0.15f});
    window.draw(title);

    // Resume
    {
        sf::Text txt(font);
        txt.setCharacterSize(fs(0.024f));
        txt.setPosition({VW * 0.38f, VH * 0.26f});
        if (selected == 0) {
            txt.setFillColor(sf::Color::Yellow);
            txt.setString("> Resume");
        } else {
            txt.setFillColor(Config::COLOR_TEXT_DEFAULT);
            txt.setString("  Resume");
        }
        window.draw(txt);
    }

    // Quit to Title
    {
        sf::Text txt(font);
        txt.setCharacterSize(fs(0.024f));
        txt.setPosition({VW * 0.38f, VH * 0.31f});
        if (selected == 1) {
            txt.setFillColor(sf::Color::Yellow);
            txt.setString("> Quit to Title");
        } else {
            txt.setFillColor(Config::COLOR_TEXT_DEFAULT);
            txt.setString("  Quit to Title");
        }
        window.draw(txt);
    }
}

} // namespace PauseMenu
