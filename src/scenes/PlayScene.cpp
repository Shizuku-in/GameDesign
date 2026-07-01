#include "scenes/PlayScene.hpp"
#include "core/Game.hpp"
#include "core/Random.hpp"
#include "data/Constants.hpp"
#include "gameplay/CharacterDefs.hpp"
#include "gameplay/EnemyDefs.hpp"
#include "graphics/SpriteSheet.hpp"
#include "scenes/GameOverScene.hpp"
#include "scenes/TitleScene.hpp"
#include "systems/CollisionSystem.hpp"
#include "ui/PauseMenu.hpp"
#include "ui/UpgradeUI.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

// ---------------------------------------------------------------------------
// 构造
// ---------------------------------------------------------------------------
PlayScene::PlayScene(Game& game) : m_game(game), m_sounds(m_game.getSounds()) {
    m_map = &MAP_DEFS[0]; // 默认森林地图
    m_spawning.setMap(*m_map);
    if (!m_tilemap.loadFromFile(m_map->tilemapPath)) {
        std::fprintf(stderr,
                     "[ERROR] PlayScene: failed to load tilemap, using fallback world size\n");
        m_worldWidth = Config::VIEW_WIDTH * 2.f;
        m_worldHeight = Config::VIEW_HEIGHT * 2.f;
    } else {
        m_worldWidth = m_tilemap.getWidth();
        m_worldHeight = m_tilemap.getHeight();
    }
    m_player.pos = {m_worldWidth / 2.f, m_worldHeight / 2.f};

    m_camera = sf::View(sf::FloatRect({0.f, 0.f}, {Config::VIEW_WIDTH, Config::VIEW_HEIGHT}));
    m_camera.setCenter(m_player.pos);

    auto fontPtr = m_game.getFonts().get("default");
    m_font = fontPtr.get();
    if (m_font != nullptr) {
        m_hud = std::make_unique<HUD>(*m_font);
    }

    Random::init();

    // 预分配
    m_enemies.reserve(Config::POOL_ENEMIES_CAPACITY);
    m_projectiles.reserve(Config::POOL_PROJECTILES_CAPACITY);
    m_xpGems.reserve(Config::POOL_XPGEMS_CAPACITY);
    m_damageTexts.reserve(Config::POOL_DAMAGETEXTS_CAPACITY);

    // 加载敌人精灵表 — 从 EnemyDef 读取路径
    for (int i = 0; i < static_cast<int>(EnemyType::Count); ++i) {
        const auto& def = ENEMY_DEFS[i];
        bool moveOk =
            m_spritesMove[i].loadFromFile(def.spriteMovePath, def.frameWidth, def.frameHeight);
        bool damagedOk = m_spritesDamaged[i].loadFromFile(def.spriteDamagedPath, def.frameWidth,
                                                          def.frameHeight);
        if (!moveOk || !damagedOk) {
            std::fprintf(stderr, "[WARN] Enemy sprite load failed: %s (move=%d, damaged=%d)\n",
                         def.name, static_cast<int>(moveOk), static_cast<int>(damagedOk));
        }
    }
    m_spawning.setEnemySprites(m_spritesMove, m_spritesDamaged);

    // 加载角色精灵表 — 从 CharacterDef 读取路径
    const auto& charDef = CHARACTER_DEFS[0]; // 默认 Elf
    m_player.initFromCharacter(charDef.hp, charDef.speed, charDef.radius, charDef.armor,
                               charDef.magnetRange);
    const char* dirPaths[K_PLAYER_DIR_COUNT] = {charDef.spriteForward, charDef.spriteBack,
                                                charDef.spriteLeft, charDef.spriteRight,
                                                charDef.spriteIdle};
    for (std::size_t i = 0; i < K_PLAYER_DIR_COUNT; ++i) {
        if (!m_playerSprites[i].loadFromFile(dirPaths[i], charDef.frameWidth,
                                             charDef.frameHeight)) {
            std::fprintf(stderr, "[WARN] Player sprite load failed: %s\n", dirPaths[i]);
        }
    }
    m_player.spriteForward = m_playerSprites.data();
    m_player.spriteBack = &m_playerSprites[1];
    m_player.spriteLeft = &m_playerSprites[2];
    m_player.spriteRight = &m_playerSprites[3];
    m_player.spriteIdle = &m_playerSprites[4];
    m_player.currentSprite = m_player.spriteIdle;

    // BGM
    if (m_bgm.openFromFile(m_map->bgmPath)) {
        m_bgm.setLooping(true);
        m_bgm.setVolume(Config::BGM_VOLUME);
        m_bgm.play();
    } else {
        std::fprintf(stderr, "[WARN] BGM load failed: %s\n", m_map->bgmPath);
    }
}

// ---------------------------------------------------------------------------
// 事件处理
// ---------------------------------------------------------------------------
void PlayScene::handleEvent(const sf::Event& event) {
    if (m_gameOver) {
        return;
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        using Key = sf::Keyboard::Key;

        // 升级暂停 — 选项选择
        if (m_paused) {
            auto count = std::ssize(m_upgradeOptions);
            if (count == 0) {
                m_paused = false;
                return;
            }
            if (kp->code == Key::Up || kp->code == Key::W) {
                m_selectedOption = (m_selectedOption - 1 + count) % count;
            } else if (kp->code == Key::Down || kp->code == Key::S) {
                m_selectedOption = (m_selectedOption + 1) % count;
            } else if (kp->code == Key::Enter || kp->code == Key::Space) {
                applyUpgrade(m_player, m_weapons, m_upgradeOptions[m_selectedOption]);
                m_paused = false;
            } else if (kp->code == Key::Num1 && count > 0) {
                applyUpgrade(m_player, m_weapons, m_upgradeOptions[0]);
                m_paused = false;
            } else if (kp->code == Key::Num2 && count > 1) {
                applyUpgrade(m_player, m_weapons, m_upgradeOptions[1]);
                m_paused = false;
            } else if (kp->code == Key::Num3 && count > 2) {
                applyUpgrade(m_player, m_weapons, m_upgradeOptions[2]);
                m_paused = false;
            }
            return;
        }

        // 菜单暂停 — Escape 切换
        if (kp->code == Key::Escape) {
            m_menuPaused = !m_menuPaused;
            m_selectedOption = 0;
            return;
        }

        if (m_menuPaused) {
            if (kp->code == Key::Up || kp->code == Key::W) {
                m_selectedOption = (m_selectedOption - 1 + 2) % 2;
            } else if (kp->code == Key::Down || kp->code == Key::S) {
                m_selectedOption = (m_selectedOption + 1) % 2;
            } else if (kp->code == Key::Enter || kp->code == Key::Space) {
                if (m_selectedOption == 0) {
                    m_menuPaused = false;
                } else {
                    m_game.changeScene(std::make_unique<TitleScene>(m_game));
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// update — 主游戏循环（60 Hz）
// ---------------------------------------------------------------------------
void PlayScene::update(sf::Time dt) {
    if (m_gameOver || m_paused || m_menuPaused) {
        return;
    }

    float dtSec = dt.asSeconds();

    // 输入 + 移动
    handleInput();
    movePlayer(dtSec);
    updatePlayerAnimation(dtSec);

    // 武器
    m_weapons.update(dtSec, m_player, m_enemies, m_projectiles, m_sounds);

    // 实体更新
    updateEnemies(dtSec);
    updateProjectiles(dtSec);
    updateXPGems(dtSec);
    updateDamageTexts(dtSec);

    // 碰撞 + 清理
    CollisionSystem::processCollisions(m_player, m_enemies, m_projectiles, m_xpGems, m_damageTexts,
                                       m_score, m_sounds, m_worldWidth, m_worldHeight);

    // 生成
    m_spawning.update(dtSec, m_gameTime, m_player.pos, m_enemies);

    // 相机
    updateCamera();

    // 死亡检查
    if (m_player.hp <= 0.f) {
        onDeath();
        return;
    }

    // 升级检查
    if (m_player.xp >= m_player.xpToNext) {
        onLevelUp();
    }

    m_gameTime += dtSec;
    m_sounds.update(dtSec);

    if (m_hud) {
        m_hud->update(m_player, m_weapons, m_gameTime);
    }
}

// ---------------------------------------------------------------------------
// 渲染
// ---------------------------------------------------------------------------
void PlayScene::render(sf::RenderWindow& window) {
    window.setView(m_camera);
    window.clear(Config::COLOR_BG_PLAY);
    m_tilemap.draw(window);
    m_worldRenderer.render(window, m_player, m_enemies, m_projectiles, m_xpGems, m_damageTexts,
                           m_font);

    window.setView(window.getDefaultView());
    if (m_hud) {
        m_hud->render(window);
    }

    if (m_paused && (m_font != nullptr)) {
        UpgradeUI::draw(window, *m_font, m_upgradeOptions, m_selectedOption);
    }
    if (m_menuPaused && (m_font != nullptr)) {
        PauseMenu::draw(window, *m_font, m_selectedOption);
    }
}

// ===========================================================================
// update 子步骤
// ===========================================================================

void PlayScene::handleInput() {
    using Key = sf::Keyboard::Key;
    auto isDown = [](Key k) { return sf::Keyboard::isKeyPressed(k); };

    m_player.vel = {0.f, 0.f};

    if (isDown(Key::Right) || isDown(Key::D)) {
        m_player.vel.x += 1.f;
    }
    if (isDown(Key::Left) || isDown(Key::A)) {
        m_player.vel.x -= 1.f;
    }
    if (isDown(Key::Down) || isDown(Key::S)) {
        m_player.vel.y += 1.f;
    }
    if (isDown(Key::Up) || isDown(Key::W)) {
        m_player.vel.y -= 1.f;
    }

    float len = std::sqrt((m_player.vel.x * m_player.vel.x) + (m_player.vel.y * m_player.vel.y));
    if (len > 0.f) {
        m_player.vel.x /= len;
        m_player.vel.y /= len;
    }
}

void PlayScene::movePlayer(float dt) {
    m_player.pos += m_player.vel * m_player.speed * dt;

    // 钳制到世界边界
    m_player.pos.x = std::max(m_player.pos.x, m_player.radius);
    m_player.pos.y = std::max(m_player.pos.y, m_player.radius);
    m_player.pos.x = std::min(m_player.pos.x, m_worldWidth - m_player.radius);
    m_player.pos.y = std::min(m_player.pos.y, m_worldHeight - m_player.radius);

    if (m_player.invincibilityTimer > 0.f) {
        m_player.invincibilityTimer -= dt;
    }
}

void PlayScene::updatePlayerAnimation(float dt) {
    // 根据移动方向选择精灵表
    const SpriteSheet* target = m_player.spriteIdle;
    if (m_player.vel.y < 0.f) {
        target = m_player.spriteBack;
    } else if (m_player.vel.y > 0.f) {
        target = m_player.spriteForward;
    } else if (m_player.vel.x < 0.f) {
        target = m_player.spriteLeft;
    } else if (m_player.vel.x > 0.f) {
        target = m_player.spriteRight;
    }

    if (target != m_player.currentSprite) {
        m_player.currentSprite = target;
        m_player.animFrame = 0;
        m_player.animTimer = 0.f;
    }

    m_player.animTimer += dt;
    if ((m_player.currentSprite != nullptr) && m_player.currentSprite->frameCount > 0 &&
        m_player.animTimer >= Config::PLAYER_ANIM_FRAME_DURATION) {
        m_player.animTimer -= Config::PLAYER_ANIM_FRAME_DURATION;
        m_player.animFrame = (m_player.animFrame + 1) % m_player.currentSprite->frameCount;
    }
}

void PlayScene::updateEnemies(float dt) {

    m_enemies.forEach([dt](Enemy& e) {
        if (e.hitFlashTimer > 0.f) {
            e.hitFlashTimer -= dt;
        }
    });
    m_enemies.forEach([&](Enemy& e) {
        // AI 移动：朝向玩家
        sf::Vector2f dir = m_player.pos - e.pos;
        float len = std::sqrt((dir.x * dir.x) + (dir.y * dir.y));
        if (len > 0.f) {
            dir.x /= len;
            dir.y /= len;
        }
        e.pos += dir * e.speed * dt;

        // 精灵动画
        const SpriteSheet* target = (e.hitFlashTimer > 0.f) ? e.spriteDamaged : e.spriteMove;
        if (target && target != e.currentSprite) {
            e.currentSprite = target;
            e.animFrame = 0;
            e.animTimer = 0.f;
        }
        e.animTimer += dt;
        if (e.currentSprite && e.currentSprite->frameCount > 0 &&
            e.animTimer >= Config::ENEMY_ANIM_FRAME_DURATION) {
            e.animTimer -= Config::ENEMY_ANIM_FRAME_DURATION;
            e.animFrame = (e.animFrame + 1) % e.currentSprite->frameCount;
        }
    });

    // 清理远离世界的敌人
    float margin = Config::VIEW_WIDTH * Config::ENEMY_CULL_MARGIN;
    m_enemies.forEachHandle([&](Pool<Enemy>::Handle h, Enemy& e) {
        if (e.pos.x < -margin || e.pos.x > m_worldWidth + margin || e.pos.y < -margin ||
            e.pos.y > m_worldHeight + margin) {
            m_enemies.release(h);
        }
    });
}

void PlayScene::updateProjectiles(float dt) {
    m_projectiles.forEach([&](Projectile& p) {
        switch (p.motion) {
        case ProjMotion::Orbit:
            p.state.orbit.angle += p.state.orbit.speed * dt;
            p.pos = m_player.pos +
                    sf::Vector2f(std::cos(p.state.orbit.angle), std::sin(p.state.orbit.angle)) *
                        p.state.orbit.radius;
            break;
        case ProjMotion::Linear:
        default:
            p.pos += p.vel * dt;
            break;
        }
        p.lifetime -= dt;
    });

    m_projectiles.forEachHandle([&](Pool<Projectile>::Handle h, Projectile& p) {
        if (p.lifetime <= 0.f) {
            m_projectiles.release(h);
        }
    });
}

void PlayScene::updateDamageTexts(float dt) {
    m_damageTexts.forEach([&](DamageText& text) {
        text.pos += text.vel * dt;
        text.lifetime -= dt;
    });
    m_damageTexts.forEachHandle([&](Pool<DamageText>::Handle h, DamageText& text) {
        if (text.lifetime <= 0.f) {
            m_damageTexts.release(h);
        }
    });
}

void PlayScene::updateXPGems(float dt) {
    m_xpGems.forEach([&](XPGem& g) {
        if (g.magnetTimer > 0.f) {
            g.magnetTimer -= dt;
            return;
        }
        sf::Vector2f dir = m_player.pos - g.pos;
        float lenSq = (dir.x * dir.x) + (dir.y * dir.y);
        if (lenSq > m_player.magnetRange * m_player.magnetRange) {
            return;
        }
        float len = std::sqrt(lenSq);
        if (len > 0.f) {
            dir.x /= len;
            dir.y /= len;
        }
        g.pos += dir * Config::XP_GEM_MAGNET_SPEED * dt;
    });
}

void PlayScene::updateCamera() {
    m_camera.setCenter(m_player.pos);

    float halfW = Config::VIEW_WIDTH / 2.f;
    float halfH = Config::VIEW_HEIGHT / 2.f;
    sf::Vector2f center = m_camera.getCenter();

    center.x = std::max(center.x, halfW);
    center.y = std::max(center.y, halfH);
    center.x = std::min(center.x, m_worldWidth - halfW);
    center.y = std::min(center.y, m_worldHeight - halfH);

    m_camera.setCenter(center);
}

void PlayScene::onLevelUp() {
    m_paused = true;
    m_player.xp -= m_player.xpToNext;
    m_player.xpToNext += Config::XP_THRESHOLD_GROWTH;
    ++m_player.level;
    m_upgradeOptions = generateUpgrades(m_player, m_weapons);
    m_selectedOption = 0;
    m_sounds.levelup();
}

void PlayScene::onDeath() {
    m_gameOver = true;
    m_game.changeScene(
        std::make_unique<GameOverScene>(m_game, m_score, m_player.level, m_gameTime));
}
