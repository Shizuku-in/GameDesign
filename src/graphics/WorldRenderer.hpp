#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "graphics/SpriteSheet.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>

/// 世界空间渲染：实体 + 玩家（地面由 TilemapRenderer 负责）。
class WorldRenderer {
public:
    /// 在世界空间绘制全部实体（需先 setView 到相机）。
    void render(sf::RenderWindow& window, const PlayerState& player, const Pool<Enemy>& enemies,
                const Pool<Projectile>& projectiles, const Pool<XPGem>& gems,
                const Pool<DamageText>& damageTexts, const sf::Font* font);

private:
    sf::VertexArray m_entityBatch{sf::PrimitiveType::Triangles};
    const SpriteSheet* m_cachedSpriteSheet = nullptr; // 缓存，避免每帧遍历查找
};
