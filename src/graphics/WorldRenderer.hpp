#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "graphics/SpriteSheet.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <optional>

/// 世界空间渲染：实体 + 玩家（地面由 TilemapRenderer 负责）。
class WorldRenderer {
public:
    /// 在世界空间绘制全部实体（需先 setView 到相机）。
    void render(sf::RenderWindow& window, const PlayerState& player, const Pool<Enemy>& enemies,
                const Pool<Projectile>& projectiles, const Pool<XPGem>& gems,
                const Pool<DamageText>& damageTexts, const sf::Font* font);

private:
    /// 无纹理实体使用的合批三角形顶点。
    sf::VertexArray m_entityBatch{sf::PrimitiveType::Triangles};
    /// 用于延迟构造精灵的首个有效精灵表缓存。
    const SpriteSheet* m_cachedSpriteSheet = nullptr;

    /// 复用的精灵对象；SFML 3 无默认构造，首次使用时延迟创建。
    std::optional<sf::Sprite> m_sprite;
    /// 复用的伤害飘字对象；首次需要字体时延迟创建。
    std::optional<sf::Text> m_text;
};
