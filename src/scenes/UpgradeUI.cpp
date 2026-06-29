#include "scenes/UpgradeUI.hpp"
#include "data/Constants.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace {
constexpr float VW = Config::VIEW_WIDTH;
constexpr float VH = Config::VIEW_HEIGHT;
int fs(float r) { return static_cast<int>(VH * r); }
} // namespace

namespace UpgradeUI {

void draw(sf::RenderWindow& window, const sf::Font& font, const std::vector<UpgradeOption>& options,
          int selected) {
    // 半透明遮罩
    sf::RectangleShape overlay({VW, VH});
    overlay.setPosition({0.f, 0.f});
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    // 标题
    sf::Text title(font);
    title.setString("Level Up!");
    title.setCharacterSize(fs(0.03f));
    title.setFillColor(sf::Color::Yellow);
    title.setPosition({VW * 0.4f, VH * 0.09f});
    window.draw(title);

    int count = static_cast<int>(options.size());
    float yBase = VH * 0.16f;
    float yStep = VH * 0.08f;

    for (int i = 0; i < count; ++i) {
        const auto& opt = options[i];
        float y = yBase + static_cast<float>(i) * yStep;

        // 选项名
        sf::Text txt(font);
        txt.setCharacterSize(fs(0.02f));
        txt.setPosition({VW * 0.25f, y});

        std::string prefix = (i == selected) ? "> " : "  ";
        txt.setString(prefix + opt.name + "  [" + std::to_string(i + 1) + "]");
        txt.setFillColor(i == selected ? sf::Color::Yellow : sf::Color(200, 200, 200));
        window.draw(txt);

        // 简短说明
        sf::Text desc(font);
        desc.setString(opt.description);
        desc.setCharacterSize(fs(0.015f));
        desc.setFillColor(sf::Color(160, 160, 160));
        desc.setPosition({VW * 0.27f, y + VH * 0.022f});
        window.draw(desc);

        // 详细数值
        if (!opt.detail.empty()) {
            sf::Text detail(font);
            detail.setString(opt.detail);
            detail.setCharacterSize(fs(0.014f));
            detail.setFillColor(sf::Color(140, 200, 140));
            detail.setPosition({VW * 0.27f, y + VH * 0.04f});
            window.draw(detail);
        }
    }

    sf::Text hint(font);
    hint.setString("Arrow keys to select, Enter to confirm, or press 1-3");
    hint.setCharacterSize(fs(0.014f));
    hint.setFillColor(sf::Color(150, 150, 150));
    hint.setPosition({VW * 0.23f, VH * 0.85f});
    window.draw(hint);
}

} // namespace UpgradeUI
