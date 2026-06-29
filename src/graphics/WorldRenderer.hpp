#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>

/// 世界空间渲染：网格背景 + 实体 + 玩家。
class WorldRenderer {
public:
    WorldRenderer();

    /// 在世界空间绘制全部内容（需先 setView 到相机）。
    void render(sf::RenderWindow& window, const PlayerState& player, const Pool<Enemy>& enemies,
                const Pool<Projectile>& projectiles, const Pool<XPGem>& gems);

private:
    sf::VertexArray m_grid{sf::PrimitiveType::Lines};
    sf::VertexArray m_entityBatch{sf::PrimitiveType::Triangles};
    void buildGrid();
};
