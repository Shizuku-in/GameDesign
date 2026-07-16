#pragma once

#include "gameplay/UpgradeDefs.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <vector>

/// 升级选择界面渲染。无状态，纯函数。
namespace UpgradeUI {

/// 绘制升级选项列表和当前选中项。
void draw(sf::RenderWindow& window, const sf::Font& font, const std::vector<UpgradeOption>& options,
          int selected);

} // namespace UpgradeUI
