#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

/// 暂停菜单渲染。无状态，纯函数。
namespace PauseMenu {

/// 绘制暂停覆盖层和当前选中的菜单项。
void draw(sf::RenderWindow& window, const sf::Font& font, int selected);

} // namespace PauseMenu
