#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

/// 暂停菜单渲染。无状态，纯函数。
namespace PauseMenu {

void draw(sf::RenderWindow& window, const sf::Font& font, int selected);

} // namespace PauseMenu
