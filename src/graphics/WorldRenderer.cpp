#include "graphics/WorldRenderer.hpp"
#include "data/Constants.hpp"
#include "graphics/SpriteSheet.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <format>

WorldRenderer::WorldRenderer() { buildGrid(); }

void WorldRenderer::render(sf::RenderWindow& window, const PlayerState& player,
                           const Pool<Enemy>& enemies, const Pool<Projectile>& projectiles,
                           const Pool<XPGem>& gems, const Pool<DamageText>& damageTexts,
                           const sf::Font* font) {
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

    // 无精灵表的敌人回退为颜色方块
    enemies.forEach([&](const Enemy& e) {
        if (!e.currentSprite) {
            sf::Color color = sf::Color::Red;
            if (e.hitFlashTimer > 0.f)
                color = sf::Color::White;
            addQuad(e.pos, e.radius, color);
        }
    });

    // 玩家精灵（在精灵层绘制，此处仅处理无精灵的后备情况）
    if (!player.currentSprite) {
        bool visible = true;
        if (player.invincibilityTimer > 0.f) {
            int flash = static_cast<int>(player.invincibilityTimer / 0.1f);
            visible = (flash % 2 == 0);
        }
        if (visible)
            addQuad(player.pos, player.radius, sf::Color::White);
    }

    // 将所有实体通过 1 次 Draw Call 提交给 GPU！
    window.draw(m_entityBatch);

    // 绘制敌人精灵
    // 缓存任意有效精灵表来构造 sf::Sprite（SFML 3.x 无默认构造）
    if (!m_cachedSpriteSheet) {
        enemies.forEach([&](const Enemy& e) {
            if (!m_cachedSpriteSheet && e.currentSprite)
                m_cachedSpriteSheet = e.currentSprite;
        });
    }
    if (m_cachedSpriteSheet) {
        sf::Sprite enemySprite(m_cachedSpriteSheet->texture);
        enemies.forEach([&](const Enemy& e) {
            const auto* ss = e.currentSprite;
            if (!ss)
                return;

            enemySprite.setTexture(ss->texture);
            enemySprite.setTextureRect(
                sf::IntRect({e.animFrame * ss->frameWidth, 0}, {ss->frameWidth, ss->frameHeight}));
            enemySprite.setOrigin({static_cast<float>(ss->frameWidth) / 2.f,
                                   static_cast<float>(ss->frameHeight) / 2.f});
            enemySprite.setPosition(e.pos);

            enemySprite.setScale({e.spriteScale, e.spriteScale});

            window.draw(enemySprite);
        });
    }

    // 绘制玩家精灵
    if (player.currentSprite && m_cachedSpriteSheet) {
        bool visible = true;
        if (player.invincibilityTimer > 0.f) {
            int flash = static_cast<int>(player.invincibilityTimer / 0.1f);
            visible = (flash % 2 == 0);
        }
        if (visible) {
            const auto* ss = player.currentSprite;
            sf::Sprite playerSprite(m_cachedSpriteSheet->texture);
            playerSprite.setTexture(ss->texture);
            playerSprite.setTextureRect(sf::IntRect({player.animFrame * ss->frameWidth, 0},
                                                    {ss->frameWidth, ss->frameHeight}));
            playerSprite.setOrigin({static_cast<float>(ss->frameWidth) / 2.f,
                                    static_cast<float>(ss->frameHeight) / 2.f});
            playerSprite.setPosition(player.pos);
            window.draw(playerSprite);
        }
    }

    // 绘制伤害飘字
    if (font) {
        sf::Text text(*font);
        text.setCharacterSize(16);
        text.setOutlineThickness(1.f);
        text.setOutlineColor(sf::Color::Black);

        damageTexts.forEach([&](const DamageText& dt) {
            text.setString(std::format("{:.0f}", dt.damage));

            // 随时间淡出
            float alphaRatio = dt.lifetime / dt.maxLifetime;
            sf::Color textColor = sf::Color::White;
            if (dt.damage >= 20.f)
                textColor = sf::Color::Yellow; // 暴击/高伤害变黄

            textColor.a = static_cast<std::uint8_t>(255 * alphaRatio);
            sf::Color outlineColor = sf::Color::Black;
            outlineColor.a = textColor.a;

            text.setFillColor(textColor);
            text.setOutlineColor(outlineColor);

            sf::FloatRect bounds = text.getLocalBounds();
            text.setPosition(
                sf::Vector2f(dt.pos.x - bounds.size.x / 2.f, dt.pos.y - bounds.size.y / 2.f));

            window.draw(text);
        });
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
