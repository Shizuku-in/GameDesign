#include "scenes/PlayScene.hpp"

#include "core/Game.hpp"
#include "core/Random.hpp"
#include "data/Constants.hpp"
#include "gameplay/CharacterDefs.hpp"
#include "gameplay/EnemyDefs.hpp"
#include "graphics/SpriteSheet.hpp"
#include "scenes/TitleScene.hpp"
#include "systems/CollisionSystem.hpp"
#include "ui/PauseMenu.hpp"
#include "ui/UpgradeUI.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
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
    const char* spritePaths[KCount] = {
        charDef.spriteForward,
        charDef.spriteBack,
        charDef.spriteSide,
        charDef.spriteIdle,
        charDef.spriteAttack,
        charDef.spriteHit,
        charDef.spriteDeath,
        // 移动中攻击/受击变体
        charDef.spriteMovingAttackForward,
        charDef.spriteMovingAttackBack,
        charDef.spriteMovingAttackSide,
        charDef.spriteMovingHitBack,
        charDef.spriteMovingHitSide,
    };
    for (std::size_t i = 0; i < KCount; ++i) {
        if (spritePaths[i] != nullptr) {
            if (!m_playerSprites[i].loadFromFile(spritePaths[i], charDef.frameWidth,
                                                 charDef.frameHeight)) {
                std::fprintf(stderr, "[WARN] Player sprite load failed: %s\n", spritePaths[i]);
            }
        }
    }

    // 赋值精灵表指针（idle/attack/hit/death 仅保留右朝向，左朝向通过翻转实现）
    m_player.spriteForward = &m_playerSprites[KForward];
    m_player.spriteBack = &m_playerSprites[KBack];
    m_player.spriteSide = &m_playerSprites[KSide];
    m_player.spriteIdle = &m_playerSprites[KIdle];
    m_player.spriteAttack = &m_playerSprites[KAttack];
    m_player.spriteHit = &m_playerSprites[KHit];
    m_player.spriteDeath = &m_playerSprites[KDeath];
    m_player.spriteMovingAttackForward = &m_playerSprites[KMovingAttackForward];
    m_player.spriteMovingAttackBack = &m_playerSprites[KMovingAttackBack];
    m_player.spriteMovingAttackSide = &m_playerSprites[KMovingAttackSide];
    m_player.spriteMovingHitBack = &m_playerSprites[KMovingHitBack];
    m_player.spriteMovingHitSide = &m_playerSprites[KMovingHitSide];

    // 默认初始朝向右侧，待机
    m_player.facingRight = true;
    m_player.currentSprite = m_player.spriteIdle;

    // 计算攻击/受击动画时长（基于精灵帧数）
    if (m_playerSprites[KAttack].frameCount > 0) {
        m_attackAnimDuration = static_cast<float>(m_playerSprites[KAttack].frameCount) *
                               Config::PLAYER_ANIM_FRAME_DURATION;
    }
    if (m_playerSprites[KHit].frameCount > 0) {
        m_hitAnimDuration = static_cast<float>(m_playerSprites[KHit].frameCount) *
                            Config::PLAYER_ANIM_FRAME_DURATION;
    }
    if (m_playerSprites[KDeath].frameCount > 0) {
        m_deathAnimDuration = static_cast<float>(m_playerSprites[KDeath].frameCount) *
                              Config::PLAYER_ANIM_FRAME_DURATION;
    }

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
    // 死亡动画播放中 — 不接受任何输入
    if (m_deathPhase == DeathPhase::Animation) {
        return;
    }

    // 死亡冻结阶段 — 仅接受重新开始 / 退出
    if (m_deathPhase == DeathPhase::Frozen) {
        if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
            using Key = sf::Keyboard::Key;
            if (kp->code == Key::Enter || kp->code == Key::Space) {
                m_game.changeScene(std::make_unique<PlayScene>(m_game));
            } else if (kp->code == Key::Escape) {
                m_game.getWindow().close();
            }
        }
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

    // 死亡动画阶段 — 仅推进死亡动画和相机缩放，冻结游戏逻辑
    if (m_deathPhase == DeathPhase::Animation) {
        updateDeathAnimation(dtSec);
        return;
    }

    // 死亡冻结阶段 — 完全停止更新
    if (m_deathPhase == DeathPhase::Frozen) {
        return;
    }

    // 输入 + 移动
    handleInput();
    movePlayer(dtSec);
    updatePlayerAnimation(dtSec);

    // 武器（返回是否有开火，触发攻击动画。动画播放中不重置，防止重叠）
    bool weaponFired = m_weapons.update(dtSec, m_player, m_enemies, m_projectiles, m_sounds);
    if (weaponFired && (m_attackAnimDuration > 0.f) && (m_player.attackAnimTimer <= 0.f)) {
        m_player.attackAnimTimer = m_attackAnimDuration;
    }

    // 实体更新
    updateEnemies(dtSec);
    updateProjectiles(dtSec);
    updateXPGems(dtSec);
    updateDamageTexts(dtSec);

    // 碰撞 + 清理（记录碰撞前 HP，用于触发受击动画）
    float hpBefore = m_player.hp;
    CollisionSystem::processCollisions(m_player, m_enemies, m_projectiles, m_xpGems, m_damageTexts,
                                       m_score, m_sounds, m_worldWidth, m_worldHeight);
    if (m_player.hp < hpBefore && m_hitAnimDuration > 0.f) {
        m_player.hitAnimTimer = m_hitAnimDuration;
    }

    // 生成
    m_spawning.update(dtSec, m_gameTime, m_player.pos, m_enemies);

    // 相机
    updateCamera();

    // 死亡检测 — 触发死亡动画序列
    if (m_player.hp <= 0.f) {
        beginDeath();
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

    // 死亡冻结阶段 — 绘制半透明暗幕 + Game Over 信息
    if (m_deathPhase == DeathPhase::Frozen) {
        renderDeathOverlay(window);
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
    if (m_player.vel.x > 0.f) {
        m_player.facingRight = true;
    } else if (m_player.vel.x < 0.f) {
        m_player.facingRight = false;
    }

    // 推进动画状态计时器
    if (m_player.attackAnimTimer > 0.f) {
        m_player.attackAnimTimer -= dt;
    }
    if (m_player.hitAnimTimer > 0.f) {
        m_player.hitAnimTimer -= dt;
    }

    bool isMoving = (m_player.vel.x != 0.f || m_player.vel.y != 0.f);

    // 选择精灵表 —— 优先级：死亡 > 受击 > 攻击 > 移动 > 待机
    // idle/attack/hit/death 仅保留右朝向，左朝向通过 WorldRenderer 翻转实现
    const SpriteSheet* target = nullptr;

    if (m_player.deathAnimTimer > 0.f) {
        target = m_player.spriteDeath;
    } else if (m_player.hitAnimTimer > 0.f) {
        // 受击：移动中优先使用 moving_hit 变体（缺少 down/forward 时回退到 stationary hit）
        if (isMoving) {
            if (m_player.vel.y < 0.f && m_player.spriteMovingHitBack != nullptr &&
                m_player.spriteMovingHitBack->frameCount > 0) {
                target = m_player.spriteMovingHitBack; // 朝上受击
            } else if (m_player.vel.x != 0.f && m_player.spriteMovingHitSide != nullptr &&
                       m_player.spriteMovingHitSide->frameCount > 0) {
                target = m_player.spriteMovingHitSide; // 侧向受击，左右翻转由 WorldRenderer 处理
            }
        }
        if (target == nullptr) {
            target = m_player.spriteHit; // 回退到 stationary hit
        }
    } else if (m_player.attackAnimTimer > 0.f) {
        // 攻击：移动中优先使用 moving_attack 变体
        if (isMoving) {
            if (m_player.vel.y > 0.f && m_player.spriteMovingAttackForward != nullptr &&
                m_player.spriteMovingAttackForward->frameCount > 0) {
                target = m_player.spriteMovingAttackForward; // 朝下开火
            } else if (m_player.vel.y < 0.f && m_player.spriteMovingAttackBack != nullptr &&
                       m_player.spriteMovingAttackBack->frameCount > 0) {
                target = m_player.spriteMovingAttackBack; // 朝上开火
            } else if (m_player.vel.x != 0.f && m_player.spriteMovingAttackSide != nullptr &&
                       m_player.spriteMovingAttackSide->frameCount > 0) {
                target = m_player.spriteMovingAttackSide; // 侧向开火，左右翻转由 WorldRenderer 处理
            }
        }
        if (target == nullptr) {
            target = m_player.spriteAttack; // 回退到 stationary attack
        }
    } else if (isMoving) {
        // 移动动画（四方向）
        if (m_player.vel.y < 0.f) {
            target = m_player.spriteBack;
        } else if (m_player.vel.y > 0.f) {
            target = m_player.spriteForward;
        } else if (m_player.vel.x != 0.f) {
            target = m_player.spriteSide; // 左右翻转由 WorldRenderer 处理
        }
    } else {
        target = m_player.spriteIdle;
    }

    // 回退：当前 target 无有效帧时尝试任意已加载的精灵
    if ((target == nullptr) || target->frameCount == 0) {
        for (std::size_t i = 0; i < KCount; ++i) {
            if (m_playerSprites[i].frameCount > 0) {
                target = &m_playerSprites[i];
                break;
            }
        }
    }
    if ((target == nullptr) || target->frameCount == 0) {
        return;
    }

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

// ===========================================================================
// 死亡动画序列
// ===========================================================================

void PlayScene::beginDeath() {
    m_deathPhase = DeathPhase::Animation;
    m_deathAnimTimer = m_deathAnimDuration;

    // 切换到死亡精灵，清除无敌闪烁以保证死亡精灵始终可见
    m_player.currentSprite = m_player.spriteDeath;
    m_player.animFrame = 0;
    m_player.animTimer = 0.f;
    m_player.invincibilityTimer = 0.f;
    m_player.deathAnimTimer = m_deathAnimDuration;

    // 无死亡精灵 — 跳过动画阶段，直接进冻结
    if (m_player.spriteDeath == nullptr || m_player.spriteDeath->frameCount <= 0) {
        m_deathPhase = DeathPhase::Frozen;
        return;
    }

    // 记录相机初态，用于缩放插值
    m_deathCameraInitialSize = m_camera.getSize();

    // 目标缩放：4× 放大（视口缩小为 1/4）
    m_deathCameraTargetSize = {m_deathCameraInitialSize.x * 0.25f,
                               m_deathCameraInitialSize.y * 0.25f};

    // 目标中心：相机下移，使玩家最终显示在屏幕上四分之二处
    float targetH = m_deathCameraTargetSize.y;
    m_deathCameraTargetCenter = {m_player.pos.x, m_player.pos.y + targetH * 0.25f};
}

void PlayScene::updateDeathAnimation(float dt) {
    m_deathAnimTimer -= dt;

    // 推进死亡精灵帧（不循环，停在最后一帧）
    const auto* deathSprite = m_player.currentSprite;
    if (deathSprite != nullptr && deathSprite->frameCount > 0) {
        m_player.animTimer += dt;
        int targetFrame = m_player.animFrame;
        while (m_player.animTimer >= Config::PLAYER_ANIM_FRAME_DURATION) {
            m_player.animTimer -= Config::PLAYER_ANIM_FRAME_DURATION;
            if (targetFrame < deathSprite->frameCount - 1) {
                ++targetFrame; // 不循环，到最后一帧停止
            }
        }
        m_player.animFrame = targetFrame;
    }

    // 相机缩放 + 中心偏移的线性插值（0 → 1）
    float t = 1.f - (m_deathAnimTimer / m_deathAnimDuration);
    t = std::clamp(t, 0.f, 1.f);

    auto lerp = [](float a, float b, float tv) { return a + (b - a) * tv; };

    m_camera.setSize({lerp(m_deathCameraInitialSize.x, m_deathCameraTargetSize.x, t),
                      lerp(m_deathCameraInitialSize.y, m_deathCameraTargetSize.y, t)});

    sf::Vector2f initialCenter = m_player.pos;
    m_camera.setCenter({lerp(initialCenter.x, m_deathCameraTargetCenter.x, t),
                        lerp(initialCenter.y, m_deathCameraTargetCenter.y, t)});

    // 动画结束 → 进入冻结阶段
    if (m_deathAnimTimer <= 0.f) {
        m_deathAnimTimer = 0.f;
        // 确保停在最后一帧
        if (deathSprite != nullptr && deathSprite->frameCount > 0) {
            m_player.animFrame = deathSprite->frameCount - 1;
        }
        m_deathPhase = DeathPhase::Frozen;
    }
}

void PlayScene::renderDeathOverlay(sf::RenderWindow& window) {
    const float VW = Config::VIEW_WIDTH;
    const float VH = Config::VIEW_HEIGHT;
    auto fs = [VH](float r) -> unsigned int { return static_cast<unsigned int>(VH * r); };

    // 50% 不透明度黑色暗幕（非全屏红色）
    sf::RectangleShape overlay({VW, VH});
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    window.draw(overlay);

    if (m_font == nullptr) {
        return;
    }

    // 所有文字水平居中，纵向从屏幕 48% 开始排列
    auto centerX = [VW](const sf::Text& t) { return (VW - t.getLocalBounds().size.x) / 2.f; };

    // "GAME OVER" 标题
    {
        sf::Text text(*m_font, "GAME OVER", fs(0.055f));
        text.setFillColor(sf::Color::Red);
        text.setPosition({centerX(text), VH * 0.48f});
        window.draw(text);
    }

    // 击败敌人数
    {
        sf::Text text(*m_font, std::format("Enemies defeated: {}", m_score), fs(0.026f));
        text.setFillColor(sf::Color::White);
        text.setPosition({centerX(text), VH * 0.56f});
        window.draw(text);
    }

    // 达到等级
    {
        sf::Text text(*m_font, std::format("Level reached: {}", m_player.level), fs(0.026f));
        text.setFillColor(sf::Color::White);
        text.setPosition({centerX(text), VH * 0.61f});
        window.draw(text);
    }

    // 存活时间
    {
        int totalSec = static_cast<int>(m_gameTime);
        sf::Text text(*m_font, std::format("Survived: {:02d}:{:02d}", totalSec / 60, totalSec % 60),
                      fs(0.026f));
        text.setFillColor(sf::Color::White);
        text.setPosition({centerX(text), VH * 0.66f});
        window.draw(text);
    }

    // 重新开始提示
    {
        sf::Text text(*m_font, "Press ENTER to play again", fs(0.022f));
        text.setFillColor(sf::Color::Yellow);
        text.setPosition({centerX(text), VH * 0.74f});
        window.draw(text);
    }

    // 退出提示
    {
        sf::Text text(*m_font, "Press ESCAPE to quit", fs(0.022f));
        text.setFillColor(sf::Color::Yellow);
        text.setPosition({centerX(text), VH * 0.78f});
        window.draw(text);
    }
}
