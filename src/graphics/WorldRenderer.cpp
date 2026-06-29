#include "graphics/WorldRenderer.hpp"
#include "data/Constants.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>

WorldRenderer::WorldRenderer() { buildGrid(); }

void WorldRenderer::render(sf::RenderWindow& window, const PlayerState& player,
                           const Pool<Enemy>& enemies, const Pool<Projectile>& projectiles,
                           const Pool<XPGem>& gems) {
    window.draw(m_grid);

    // XP 宝石（绿色）
    gems.forEach([&](const XPGem& g) {
        sf::CircleShape shape(g.radius);
        shape.setPosition({g.pos.x - g.radius, g.pos.y - g.radius});
        shape.setFillColor(sf::Color::Green);
        window.draw(shape);
    });

    // 弹幕（黄色）
    projectiles.forEach([&](const Projectile& p) {
        sf::CircleShape shape(p.radius);
        shape.setPosition({p.pos.x - p.radius, p.pos.y - p.radius});
        shape.setFillColor(sf::Color::Yellow);
        window.draw(shape);
    });

    // 敌人（红色系）
    enemies.forEach([&](const Enemy& e) {
        sf::CircleShape shape(e.radius);
        shape.setPosition({e.pos.x - e.radius, e.pos.y - e.radius});
        switch (e.type) {
        case EnemyType::Basic:
            shape.setFillColor(Config::COLOR_ENEMY_BASIC);
            break;
        case EnemyType::Fast:
            shape.setFillColor(Config::COLOR_ENEMY_FAST);
            break;
        case EnemyType::Tank:
            shape.setFillColor(Config::COLOR_ENEMY_TANK);
            break;
        case EnemyType::Boss:
            shape.setFillColor(Config::COLOR_ENEMY_BOSS);
            break;
        default:
            shape.setFillColor(sf::Color::Red);
        }
        window.draw(shape);
    });

    // 玩家（白色，无敌时闪烁）
    bool visible = true;
    if (player.invincibilityTimer > 0.f) {
        int flash = static_cast<int>(player.invincibilityTimer / 0.1f);
        visible = (flash % 2 == 0);
    }
    if (visible) {
        sf::CircleShape shape(player.radius);
        shape.setPosition({player.pos.x - player.radius, player.pos.y - player.radius});
        shape.setFillColor(sf::Color::White);
        window.draw(shape);
    }
}

void WorldRenderer::buildGrid() {
    constexpr float CELL = Config::VIEW_WIDTH / 24.f; // 约 80px @1920
    constexpr float W = Config::WORLD_WIDTH;
    constexpr float H = Config::WORLD_HEIGHT;
    sf::Color lineColor(50, 50, 50);

    m_grid.clear();

    auto addVertex = [&](float x, float y) {
        sf::Vertex v;
        v.position = {x, y};
        v.color = lineColor;
        m_grid.append(v);
    };

    for (float x = 0.f; x <= W; x += CELL) {
        addVertex(x, 0.f);
        addVertex(x, H);
    }
    for (float y = 0.f; y <= H; y += CELL) {
        addVertex(0.f, y);
        addVertex(W, y);
    }
}
