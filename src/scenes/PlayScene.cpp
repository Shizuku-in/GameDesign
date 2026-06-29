#include "scenes/PlayScene.hpp"
#include "core/Game.hpp"
#include "data/Constants.hpp"
#include "scenes/GameOverScene.hpp"
#include "scenes/PauseMenu.hpp"
#include "scenes/TitleScene.hpp"
#include "scenes/UpgradeUI.hpp"
#include "systems/CollisionSystem.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <cmath>
#include <cstdlib>
#include <ctime>

// ---------------------------------------------------------------------------
// 构造
// ---------------------------------------------------------------------------
PlayScene::PlayScene(Game& game) : m_game(game), m_sounds(m_game.getSounds()) {
    m_camera = sf::View(sf::FloatRect({0.f, 0.f}, {Config::VIEW_WIDTH, Config::VIEW_HEIGHT}));
    m_camera.setCenter(m_player.pos);

    auto fontPtr = m_game.getFonts().get("default");
    m_font = fontPtr.get();
    if (m_font)
        m_hud = std::make_unique<HUD>(*m_font);

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // 预分配
    m_enemies.reserve(Config::POOL_ENEMIES_CAPACITY);
    m_projectiles.reserve(Config::POOL_PROJECTILES_CAPACITY);
    m_xpGems.reserve(Config::POOL_XPGEMS_CAPACITY);

    // BGM
    if (m_bgm.openFromFile(Config::BGM_PLAY_SCENE_PATH)) {
        m_bgm.setLooping(true);
        m_bgm.setVolume(50.f);
        m_bgm.play();
    }
}

// ---------------------------------------------------------------------------
// 事件处理
// ---------------------------------------------------------------------------
void PlayScene::handleEvent(const sf::Event& event) {
    if (m_gameOver)
        return;

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        using Key = sf::Keyboard::Key;

        // 升级暂停 — 选项选择
        if (m_paused) {
            int count = static_cast<int>(m_upgradeOptions.size());
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
    if (m_gameOver || m_paused || m_menuPaused)
        return;

    float dtSec = dt.asSeconds();

    // 输入 + 移动
    handleInput();
    movePlayer(dtSec);

    // 武器
    m_weapons.update(dtSec, m_player, m_enemies, m_projectiles, m_sounds);

    // 实体更新
    updateEnemies(dtSec);
    updateProjectiles(dtSec);
    updateXPGems(dtSec);

    // 碰撞 + 清理
    CollisionSystem::processCollisions(m_player, m_enemies, m_projectiles, m_xpGems, m_score,
                                       m_sounds);

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
    if (m_player.xp >= m_player.xpToNext)
        onLevelUp();

    m_gameTime += dtSec;
    m_sounds.update(dtSec);

    if (m_hud)
        m_hud->update(m_player, m_weapons, m_gameTime);
}

// ---------------------------------------------------------------------------
// 渲染
// ---------------------------------------------------------------------------
void PlayScene::render(sf::RenderWindow& window) {
    window.setView(m_camera);
    window.clear(Config::COLOR_BG_PLAY);
    m_worldRenderer.render(window, m_player, m_enemies, m_projectiles, m_xpGems);

    window.setView(window.getDefaultView());
    if (m_hud)
        m_hud->render(window);

    if (m_paused && m_font)
        UpgradeUI::draw(window, *m_font, m_upgradeOptions, m_selectedOption);
    if (m_menuPaused && m_font)
        PauseMenu::draw(window, *m_font, m_selectedOption);
}

// ===========================================================================
// update 子步骤
// ===========================================================================

void PlayScene::handleInput() {
    using Key = sf::Keyboard::Key;
    auto isDown = [](Key k) { return sf::Keyboard::isKeyPressed(k); };

    m_player.vel = {0.f, 0.f};

    if (isDown(Key::Right) || isDown(Key::D))
        m_player.vel.x += 1.f;
    if (isDown(Key::Left) || isDown(Key::A))
        m_player.vel.x -= 1.f;
    if (isDown(Key::Down) || isDown(Key::S))
        m_player.vel.y += 1.f;
    if (isDown(Key::Up) || isDown(Key::W))
        m_player.vel.y -= 1.f;

    float len = std::sqrt(m_player.vel.x * m_player.vel.x + m_player.vel.y * m_player.vel.y);
    if (len > 0.f) {
        m_player.vel.x /= len;
        m_player.vel.y /= len;
    }
}

void PlayScene::movePlayer(float dt) {
    m_player.pos += m_player.vel * m_player.speed * dt;

    // 钳制到世界边界
    if (m_player.pos.x < m_player.radius)
        m_player.pos.x = m_player.radius;
    if (m_player.pos.y < m_player.radius)
        m_player.pos.y = m_player.radius;
    if (m_player.pos.x > Config::WORLD_WIDTH - m_player.radius)
        m_player.pos.x = Config::WORLD_WIDTH - m_player.radius;
    if (m_player.pos.y > Config::WORLD_HEIGHT - m_player.radius)
        m_player.pos.y = Config::WORLD_HEIGHT - m_player.radius;

    if (m_player.invincibilityTimer > 0.f)
        m_player.invincibilityTimer -= dt;
}

void PlayScene::updateEnemies(float dt) {
    m_enemies.forEach([&](Enemy& e) {
        sf::Vector2f dir = m_player.pos - e.pos;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.f) {
            dir.x /= len;
            dir.y /= len;
        }
        e.pos += dir * e.speed * dt;
    });

    // 清理远离世界的敌人
    float margin = Config::VIEW_WIDTH * Config::ENEMY_CULL_MARGIN;
    m_enemies.forEachHandle([&](Pool<Enemy>::Handle h, Enemy& e) {
        if (e.pos.x < -margin || e.pos.x > Config::WORLD_WIDTH + margin || e.pos.y < -margin ||
            e.pos.y > Config::WORLD_HEIGHT + margin) {
            m_enemies.release(h);
        }
    });
}

void PlayScene::updateProjectiles(float dt) {
    m_projectiles.forEach([&](Projectile& p) {
        if (p.orbitRadius > 0.f) {
            p.orbitAngle += p.orbitSpeed * dt;
            p.pos = m_player.pos +
                    sf::Vector2f(std::cos(p.orbitAngle), std::sin(p.orbitAngle)) * p.orbitRadius;
        } else {
            p.pos += p.vel * dt;
        }
        p.lifetime -= dt;
    });

    m_projectiles.forEachHandle([&](Pool<Projectile>::Handle h, Projectile& p) {
        if (p.lifetime <= 0.f)
            m_projectiles.release(h);
    });
}

void PlayScene::updateXPGems(float dt) {
    m_xpGems.forEach([&](XPGem& g) {
        if (g.magnetTimer > 0.f) {
            g.magnetTimer -= dt;
            return;
        }
        sf::Vector2f dir = m_player.pos - g.pos;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
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

    if (center.x < halfW)
        center.x = halfW;
    if (center.y < halfH)
        center.y = halfH;
    if (center.x > Config::WORLD_WIDTH - halfW)
        center.x = Config::WORLD_WIDTH - halfW;
    if (center.y > Config::WORLD_HEIGHT - halfH)
        center.y = Config::WORLD_HEIGHT - halfH;

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
