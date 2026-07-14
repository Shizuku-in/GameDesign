#include "graphics/WorldRenderer.hpp"

#include "data/Constants.hpp"
#include "graphics/SpriteSheet.hpp"

#include <SFML/Graphics/Color.hpp>

#include <format>

void WorldRenderer::render(sf::RenderWindow& window, const PlayerState& player,
                           const Pool<Enemy>& enemies, const Pool<Projectile>& projectiles,
                           const Pool<XPGem>& gems, const Pool<DamageText>& damageTexts,
                           const sf::Font* font) {
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
            if (e.hitFlashTimer > 0.f) {
                color = sf::Color::White;
            } else if (e.frozenTimer > 0.f) {
                color = sf::Color::Cyan; // 冻结时变青色
            }
            addQuad(e.pos, e.radius, color);
        }
    });

    // 玩家精灵（在精灵层绘制，此处仅处理无精灵的后备情况）
    if (player.currentSprite == nullptr) {
        addQuad(player.pos, player.radius, sf::Color::White);
    }

    // 将所有实体通过 1 次 Draw Call 提交给 GPU！
    window.draw(m_entityBatch);

    // 绘制敌人精灵
    // 缓存任意有效精灵表来构造 sf::Sprite（SFML 3.x 无默认构造）
    if (m_cachedSpriteSheet == nullptr) {
        enemies.forEach([&](const Enemy& e) {
            if (!m_cachedSpriteSheet && e.currentSprite) {
                m_cachedSpriteSheet = e.currentSprite;
            }
        });
    }
    if (m_cachedSpriteSheet != nullptr) {
        if (!m_sprite.has_value()) {
            m_sprite.emplace(m_cachedSpriteSheet->texture);
        }
        enemies.forEach([&](const Enemy& e) {
            const auto* ss = e.currentSprite;
            if (!ss) {
                return;
            }

            auto& sprite = *m_sprite;
            sprite.setTexture(ss->texture);
            sprite.setTextureRect(
                sf::IntRect({e.animFrame * ss->frameWidth, 0}, {ss->frameWidth, ss->frameHeight}));
            sprite.setOrigin({static_cast<float>(ss->frameWidth) / 2.f,
                              static_cast<float>(ss->frameHeight) / 2.f});
            sprite.setPosition(e.pos);

            // 应用朝向缩放
            float xScale = e.facingRight ? e.spriteScale : -e.spriteScale;
            sprite.setScale({xScale, e.spriteScale});

            if (e.frozenTimer > 0.f) {
                // 冻结时给精灵添加蓝色调
                sprite.setColor(sf::Color(150, 200, 255));
            } else {
                sprite.setColor(sf::Color::White);
            }

            window.draw(sprite);
        });
    }

    // 绘制玩家精灵（直接用玩家自己的纹理，不依赖敌人缓存）
    if (player.currentSprite != nullptr) {
        const auto* ss = player.currentSprite;
        if (!m_sprite.has_value()) {
            m_sprite.emplace(ss->texture);
        }
        auto& sprite = *m_sprite;
        sprite.setTexture(ss->texture);
        sprite.setTextureRect(
            sf::IntRect({player.animFrame * ss->frameWidth, 0}, {ss->frameWidth, ss->frameHeight}));
        sprite.setOrigin(
            {static_cast<float>(ss->frameWidth) / 2.f, static_cast<float>(ss->frameHeight) / 2.f});
        sprite.setPosition(player.pos);
        float xScale = player.facingRight ? 1.f : -1.f;
        sprite.setScale({xScale, 1.f});
        window.draw(sprite);
    }

    // 绘制伤害飘字
    if (font != nullptr) {
        if (!m_text.has_value()) {
            m_text.emplace(*font);
            m_text->setCharacterSize(16);
            m_text->setOutlineThickness(1.f);
            m_text->setOutlineColor(sf::Color::Black);
        }

        auto& text = *m_text;
        damageTexts.forEach([&](const DamageText& dt) {
            text.setString(std::format("{:.0f}", dt.damage));

            // 随时间淡出
            float alphaRatio = dt.lifetime / dt.maxLifetime;
            sf::Color textColor = sf::Color::White;
            if (dt.damage >= 20.f) {
                textColor = sf::Color::Yellow; // 暴击/高伤害变黄
            }

            textColor.a = static_cast<std::uint8_t>(255 * alphaRatio);
            sf::Color outlineColor = sf::Color::Black;
            outlineColor.a = textColor.a;

            text.setFillColor(textColor);
            text.setOutlineColor(outlineColor);

            sf::FloatRect bounds = text.getLocalBounds();
            text.setPosition(
                sf::Vector2f(dt.pos.x - (bounds.size.x / 2.f), dt.pos.y - (bounds.size.y / 2.f)));

            window.draw(text);
        });
    }
}
