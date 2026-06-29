#include "graphics/WorldRenderer.hpp"
#include "data/Constants.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>

WorldRenderer::WorldRenderer() { buildGrid(); }

void WorldRenderer::render(sf::RenderWindow& window, const PlayerState& player,
                           const Pool<Enemy>& enemies, const Pool<Projectile>& projectiles,
                           const Pool<XPGem>& gems) {
    window.draw(m_grid);

    // 清空上一帧的批处理顶点
    m_entityBatch.clear();

    // 辅助函数：追加一个由 2 个三角形（6 个顶点）组成的矩形
    auto addQuad = [&](sf::Vector2f center, float radius, sf::Color color) {
        sf::Vertex tl{{center.x - radius, center.y - radius}, color};
        sf::Vertex tr{{center.x + radius, center.y - radius}, color};
        sf::Vertex bl{{center.x - radius, center.y + radius}, color};
        sf::Vertex br{{center.x + radius, center.y + radius}, color};

        m_entityBatch.append(tl);
        m_entityBatch.append(tr);
        m_entityBatch.append(bl);

        m_entityBatch.append(tr);
        m_entityBatch.append(br);
        m_entityBatch.append(bl);
    };

    // XP 宝石（绿色）
    gems.forEach([&](const XPGem& g) { addQuad(g.pos, g.radius, sf::Color::Green); });

    // 弹幕（黄色）
    projectiles.forEach([&](const Projectile& p) { addQuad(p.pos, p.radius, sf::Color::Yellow); });

    // 敌人（红色系）
    enemies.forEach([&](const Enemy& e) {
        sf::Color color;
        switch (e.type) {
        case EnemyType::Basic:
            color = Config::COLOR_ENEMY_BASIC;
            break;
        case EnemyType::Fast:
            color = Config::COLOR_ENEMY_FAST;
            break;
        case EnemyType::Tank:
            color = Config::COLOR_ENEMY_TANK;
            break;
        case EnemyType::Boss:
            color = Config::COLOR_ENEMY_BOSS;
            break;
        default:
            color = sf::Color::Red;
        }
        addQuad(e.pos, e.radius, color);
    });

    // 玩家（白色，无敌时闪烁）
    bool visible = true;
    if (player.invincibilityTimer > 0.f) {
        int flash = static_cast<int>(player.invincibilityTimer / 0.1f);
        visible = (flash % 2 == 0);
    }
    if (visible) {
        addQuad(player.pos, player.radius, sf::Color::White);
    }

    // 将所有实体通过 1 次 Draw Call 提交给 GPU！
    window.draw(m_entityBatch);
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
