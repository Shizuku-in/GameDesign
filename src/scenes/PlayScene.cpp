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
                               charDef.magnetRange, charDef.damageBonus, charDef.cooldownReduction);

    // 构造路径数组（nullptr = 无此精灵，跳过加载）
    const char* spritePaths[kCount] = {
        charDef.spriteForward, charDef.spriteBack, charDef.spriteSide,  charDef.spriteIdle,
        charDef.spriteAttack,  charDef.spriteHit,  charDef.spriteDeath,
    };
    for (std::size_t i = 0; i < kCount; ++i) {
        if (spritePaths[i]) {
            if (!m_playerSprites[i].loadFromFile(spritePaths[i], charDef.frameWidth,
                                                 charDef.frameHeight))
                std::fprintf(stderr, "[WARN] Player sprite load failed: %s\n", spritePaths[i]);
        }
    }

    // 赋值精灵表指针（idle/attack/hit 仅保留右朝向，左朝向通过翻转实现）
    m_player.spriteForward = &m_playerSprites[kForward];
    m_player.spriteBack = &m_playerSprites[kBack];
    m_player.spriteSide = &m_playerSprites[kSide];
    m_player.spriteIdle = &m_playerSprites[kIdle];
    m_player.spriteAttack = &m_playerSprites[kAttack];
    m_player.spriteHit = &m_playerSprites[kHit];
    m_player.spriteDeath = &m_playerSprites[kDeath];

    // 默认初始朝向右侧，待机
    m_player.facingRight = true;
    m_player.currentSprite = m_player.spriteIdle;

    // 计算攻击/受击动画时长（基于精灵帧数）
    if (m_playerSprites[kAttack].frameCount > 0)
        m_attackAnimDuration = static_cast<float>(m_playerSprites[kAttack].frameCount) *
                               Config::PLAYER_ANIM_FRAME_DURATION;
    if (m_playerSprites[kHit].frameCount > 0)
        m_hitAnimDuration = static_cast<float>(m_playerSprites[kHit].frameCount) *
                            Config::PLAYER_ANIM_FRAME_DURATION;
    if (m_playerSprites[kDeath].frameCount > 0)
        m_deathAnimDuration = static_cast<float>(m_playerSprites[kDeath].frameCount) *
                              Config::PLAYER_ANIM_FRAME_DURATION;

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
    if (m_paused || m_menuPaused) {
        return;
    }

    float dtSec = dt.asSeconds();

    // 死亡动画播放中：仅更新动画和相机，冻结游戏逻辑
    if (m_gameOver) {
        m_player.deathAnimTimer -= dtSec;
        updatePlayerAnimation(dtSec);
        updateCamera();
        if (m_player.deathAnimTimer <= 0.f) {
            m_game.changeScene(
                std::make_unique<GameOverScene>(m_game, m_score, m_player.level, m_gameTime));
        }
        return;
    }

    // 输入 + 移动
    handleInput();
    movePlayer(dtSec);
    updatePlayerAnimation(dtSec);

    // 武器（返回是否有开火，触发攻击动画。动画播放中不重置，防止重叠）
    bool weaponFired = m_weapons.update(dtSec, m_player, m_enemies, m_projectiles, m_sounds);
    if (weaponFired && (m_attackAnimDuration > 0.f) && (m_player.attackAnimTimer <= 0.f))
        m_player.attackAnimTimer = m_attackAnimDuration;

    // 实体更新
    updateEnemies(dtSec);
    updateProjectiles(dtSec);
    updateXPGems(dtSec);
    updateDamageTexts(dtSec);

    // 碰撞 + 清理（记录碰撞前 HP，用于触发受击动画）
    float hpBefore = m_player.hp;
    CollisionSystem::processCollisions(m_player, m_enemies, m_projectiles, m_xpGems, m_damageTexts,
                                       m_score, m_sounds, m_worldWidth, m_worldHeight);
    if (m_player.hp < hpBefore && m_hitAnimDuration > 0.f)
        m_player.hitAnimTimer = m_hitAnimDuration;

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
    // 更新朝向（仅水平移动改变 facing，垂直移动/静止保持上次朝向）
    if (m_player.vel.x > 0.f)
        m_player.facingRight = true;
    else if (m_player.vel.x < 0.f)
        m_player.facingRight = false;

    // 推进动画状态计时器
    if (m_player.attackAnimTimer > 0.f)
        m_player.attackAnimTimer -= dt;
    if (m_player.hitAnimTimer > 0.f)
        m_player.hitAnimTimer -= dt;

    bool isMoving = (m_player.vel.x != 0.f || m_player.vel.y != 0.f);

    // 选择精灵表 —— 优先级：死亡 > 受击 > 攻击 > 移动 > 待机
    // idle/attack/hit/death 仅保留右朝向，左朝向通过 WorldRenderer 翻转实现
    const SpriteSheet* target = nullptr;

    if (m_player.deathAnimTimer > 0.f) {
        target = m_player.spriteDeath;
    } else if (m_player.hitAnimTimer > 0.f) {
        target = m_player.spriteHit;
    } else if (m_player.attackAnimTimer > 0.f) {
        target = m_player.spriteAttack;
    } else if (isMoving) {
        // 移动动画（四方向）
        if (m_player.vel.y < 0.f)
            target = m_player.spriteBack;
        else if (m_player.vel.y > 0.f)
            target = m_player.spriteForward;
        else if (m_player.vel.x != 0.f)
            target = m_player.spriteSide; // 左右翻转由 WorldRenderer 处理
    } else {
        target = m_player.spriteIdle;
    }

    // 回退：当前 target 无有效帧时尝试任意已加载的精灵
    if (!target || target->frameCount == 0) {
        for (std::size_t i = 0; i < kCount; ++i) {
            if (m_playerSprites[i].frameCount > 0) {
                target = &m_playerSprites[i];
                break;
            }
        }
    }
    if (!target || target->frameCount == 0)
        return;

    // 精灵表切换时重置帧
    if (target != m_player.currentSprite) {
        m_player.currentSprite = target;
        m_player.animFrame = 0;
        m_player.animTimer = 0.f;
    }

    // 推进帧动画
    m_player.animTimer += dt;
    if ((m_player.currentSprite != nullptr) && m_player.currentSprite->frameCount > 0 &&
        m_player.animTimer >= Config::PLAYER_ANIM_FRAME_DURATION) {
        m_player.animTimer -= Config::PLAYER_ANIM_FRAME_DURATION;
        m_player.animFrame = (m_player.animFrame + 1) % m_player.currentSprite->frameCount;
    }
}

void PlayScene::updateEnemies(float dt) {

    m_enemies.forEach([&](Enemy& e) {
        // 闪白计时器
        if (e.hitFlashTimer > 0.f) {
            e.hitFlashTimer -= dt;
        }

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
    m_player.deathAnimTimer = m_deathAnimDuration;
    // 立即切换到死亡精灵
    m_player.currentSprite = m_player.spriteDeath;
    m_player.animFrame = 0;
    m_player.animTimer = 0.f;
}
