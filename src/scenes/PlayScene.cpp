#include "scenes/PlayScene.hpp"
#include "core/Game.hpp"
#include "data/Collision.hpp"
#include "data/Constants.hpp"
#include "scenes/GameOverScene.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <cmath>
#include <cstdlib>
#include <ctime>

namespace {
float randFloat() { return static_cast<float>(std::rand()) / RAND_MAX; }
} // namespace

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
PlayScene::PlayScene(Game& game)
    : m_game(game), m_spawnInterval(Config::ENEMY_BASE_SPAWN_INTERVAL),
      m_enemiesPerWave(Config::ENEMIES_PER_WAVE_BASE), m_bossTimer(60.f) // first boss at 60s
{
    m_camera = sf::View(sf::FloatRect({0.f, 0.f}, {Config::VIEW_WIDTH, Config::VIEW_HEIGHT}));
    m_camera.setCenter(m_player.pos);

    auto fontPtr = m_game.getFonts().get("default");
    m_font = fontPtr.get();
    if (m_font)
        m_hud = std::make_unique<HUD>(*m_font);

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Pre-allocate to avoid reallocation during gameplay
    m_enemies.reserve(500);
    m_projectiles.reserve(200);
    m_xpGems.reserve(300);
}

// ---------------------------------------------------------------------------
// handleEvent
// ---------------------------------------------------------------------------
void PlayScene::handleEvent(const sf::Event& event) {
    if (m_gameOver)
        return;

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        using Key = sf::Keyboard::Key;

        if (kp->code == Key::Escape) {
            m_game.getWindow().close();
            return;
        }

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
        }
    }
}

// ---------------------------------------------------------------------------
// update — main gameplay loop (60 Hz)
// ---------------------------------------------------------------------------
void PlayScene::update(sf::Time dt) {
    if (m_gameOver)
        return;

    float dtSec = dt.asSeconds();

    if (m_paused)
        return;

    // 1. Input + movement
    handleInput();
    movePlayer(dtSec);

    // 2. Weapons
    m_weapons.update(dtSec, m_player, m_enemies, m_projectiles);

    // 3. Enemy AI
    updateEnemies(dtSec);

    // 4. Projectile movement + orbit
    updateProjectiles(dtSec);

    // 5. XP gem movement + magnet
    updateXPGems(dtSec);

    // 6. Collisions (with cleanup)
    checkCollisions(dtSec);

    // 7. Spawn waves
    updateSpawning(dtSec);

    // 8. Camera
    updateCamera();

    // 9. Death check (before level-up)
    if (m_player.hp <= 0.f) {
        onDeath();
        return;
    }

    // 10. Level-up check
    if (m_player.xp >= m_player.xpToNext) {
        onLevelUp();
    }

    m_gameTime += dtSec;

    if (m_hud)
        m_hud->update(m_player, m_weapons, m_gameTime);
}

// ---------------------------------------------------------------------------
// render
// ---------------------------------------------------------------------------
void PlayScene::render(sf::RenderWindow& window) {
    window.setView(m_camera);
    window.clear(sf::Color(30, 30, 30));
    drawEntities(window);

    window.setView(window.getDefaultView());
    if (m_font)
        m_hud->render(window);

    if (m_paused)
        drawUpgradeUI(window);
}

// ===========================================================================
// update sub-steps
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

    // Clamp to world
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

    // Clean up enemies far outside the world
    m_enemies.forEachHandle([&](Pool<Enemy>::Handle h, Enemy& e) {
        if (e.pos.x < -300.f || e.pos.x > Config::WORLD_WIDTH + 300.f || e.pos.y < -300.f ||
            e.pos.y > Config::WORLD_HEIGHT + 300.f) {
            m_enemies.release(h);
        }
    });
}

void PlayScene::updateProjectiles(float dt) {
    m_projectiles.forEach([&](Projectile& p) {
        if (p.orbitRadius > 0.f) {
            // Orbiting projectile (Axe)
            p.orbitAngle += p.orbitSpeed * dt;
            p.pos = m_player.pos +
                    sf::Vector2f(std::cos(p.orbitAngle), std::sin(p.orbitAngle)) * p.orbitRadius;
        } else {
            p.pos += p.vel * dt;
        }
        p.lifetime -= dt;
    });

    // Clean up expired projectiles
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
        // Magnet pull
        sf::Vector2f dir = m_player.pos - g.pos;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.f) {
            dir.x /= len;
            dir.y /= len;
        }
        g.pos += dir * Config::XP_GEM_MAGNET_SPEED * dt;
    });
}

void PlayScene::checkCollisions(float /*dt*/) {
    // Projectile vs Enemy
    m_projectiles.forEach([&](Projectile& p) {
        m_enemies.forEach([&](Enemy& e) {
            if (e.hp <= 0.f)
                return;
            if (circleCircle(p.pos, p.radius, e.pos, e.radius)) {
                e.hp -= p.damage;
                --p.pierceCount;
                if (p.pierceCount < 0)
                    p.lifetime = 0.f;
                if (e.hp <= 0.f) {
                    spawnGem(e.pos, e.xpValue);
                    ++m_score;
                }
            }
        });
    });

    // Player vs Enemy
    if (m_player.invincibilityTimer <= 0.f) {
        m_enemies.forEach([&](Enemy& e) {
            if (e.hp <= 0.f)
                return;
            if (circleCircle(m_player.pos, m_player.radius, e.pos, e.radius)) {
                float dmg = e.damage * (1.f / 60.f) * (1.f - m_player.armor);
                m_player.hp -= dmg;
                m_player.invincibilityTimer = Config::PLAYER_IFRAMES;
            }
        });
    }

    // Player vs XP Gem
    m_xpGems.forEach([&](XPGem& g) {
        if (circleCircle(m_player.pos, m_player.magnetRange, g.pos, g.radius)) {
            m_player.xp += g.value * m_player.xpMultiplier;
            g.value = -1.f; // mark for cleanup
        }
    });

    // --- Cleanup dead/already-processed entities ---

    m_enemies.forEachHandle([&](Pool<Enemy>::Handle h, Enemy& e) {
        if (e.hp <= 0.f)
            m_enemies.release(h);
    });

    m_projectiles.forEachHandle([&](Pool<Projectile>::Handle h, Projectile& p) {
        if (p.lifetime <= 0.f)
            m_projectiles.release(h);
    });

    m_xpGems.forEachHandle([&](Pool<XPGem>::Handle h, XPGem& g) {
        if (g.value <= 0.f)
            m_xpGems.release(h);
    });
}

void PlayScene::updateSpawning(float dt) {
    m_spawnTimer -= dt;
    m_difficultyTimer += dt;
    m_bossTimer -= dt;

    // Boss every 60s
    if (m_bossTimer <= 0.f) {
        spawnEnemy(EnemyType::Boss);
        m_bossTimer = 60.f;
    }

    if (m_spawnTimer > 0.f)
        return;

    // Available enemy types based on elapsed time
    int activeTypes = 1;
    if (m_gameTime > Config::ENEMY_APPEAR_TIME[1])
        activeTypes = 2;
    if (m_gameTime > Config::ENEMY_APPEAR_TIME[2])
        activeTypes = 3;

    for (int i = 0; i < m_enemiesPerWave; ++i) {
        float totalWeight = 0.f;
        for (int t = 0; t < activeTypes; ++t)
            totalWeight += Config::ENEMY_SPAWN_WEIGHT[t];

        float r = randFloat() * totalWeight;
        float accum = 0.f;
        EnemyType chosen = EnemyType::Basic;
        for (int t = 0; t < activeTypes; ++t) {
            accum += Config::ENEMY_SPAWN_WEIGHT[t];
            if (r <= accum) {
                chosen = static_cast<EnemyType>(t);
                break;
            }
        }
        spawnEnemy(chosen);
    }

    // Ramp difficulty
    m_spawnInterval = Config::ENEMY_BASE_SPAWN_INTERVAL - m_difficultyTimer * 0.005f;
    if (m_spawnInterval < Config::ENEMY_MIN_SPAWN_INTERVAL)
        m_spawnInterval = Config::ENEMY_MIN_SPAWN_INTERVAL;

    m_spawnTimer = m_spawnInterval;
    m_enemiesPerWave = Config::ENEMIES_PER_WAVE_BASE + static_cast<int>(m_difficultyTimer / 10.f);
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
}

void PlayScene::onDeath() {
    m_gameOver = true;
    m_game.changeScene(
        std::make_unique<GameOverScene>(m_game, m_score, m_player.level, m_gameTime));
}

// ===========================================================================
// Spawn helpers
// ===========================================================================

void PlayScene::spawnEnemy(EnemyType type) {
    int t = static_cast<int>(type);
    auto handle = m_enemies.acquire();
    auto* e = m_enemies.get(handle);
    if (!e)
        return;

    e->pos = randomSpawnPosition();
    e->hp = Config::ENEMY_HP[t];
    e->maxHp = Config::ENEMY_HP[t];
    e->speed = Config::ENEMY_SPEED[t];
    e->radius = Config::ENEMY_RADIUS[t];
    e->damage = Config::ENEMY_DAMAGE[t];
    e->xpValue = Config::ENEMY_XP[t];
    e->type = type;
}

void PlayScene::spawnGem(sf::Vector2f pos, float value) {
    auto handle = m_xpGems.acquire();
    auto* g = m_xpGems.get(handle);
    if (!g)
        return;

    g->pos = pos;
    g->value = value;
    g->radius = 5.f;
    g->magnetTimer = Config::XP_GEM_MAGNET_DELAY;
}

sf::Vector2f PlayScene::randomSpawnPosition() const {
    float angle = randFloat() * 2.f * 3.14159265f;
    float dist = Config::ENEMY_SPAWN_DISTANCE + randFloat() * 200.f;
    return m_player.pos + sf::Vector2f(std::cos(angle) * dist, std::sin(angle) * dist);
}

// ===========================================================================
// Drawing
// ===========================================================================

void PlayScene::drawEntities(sf::RenderWindow& window) {
    m_xpGems.forEach([&](const XPGem& g) {
        sf::CircleShape shape(g.radius);
        shape.setPosition({g.pos.x - g.radius, g.pos.y - g.radius});
        shape.setFillColor(sf::Color::Green);
        window.draw(shape);
    });

    m_projectiles.forEach([&](const Projectile& p) {
        sf::CircleShape shape(p.radius);
        shape.setPosition({p.pos.x - p.radius, p.pos.y - p.radius});
        shape.setFillColor(sf::Color::Yellow);
        window.draw(shape);
    });

    m_enemies.forEach([&](const Enemy& e) {
        sf::CircleShape shape(e.radius);
        shape.setPosition({e.pos.x - e.radius, e.pos.y - e.radius});
        switch (e.type) {
        case EnemyType::Basic:
            shape.setFillColor(sf::Color::Red);
            break;
        case EnemyType::Fast:
            shape.setFillColor(sf::Color(255, 100, 100));
            break;
        case EnemyType::Tank:
            shape.setFillColor(sf::Color(180, 0, 0));
            break;
        case EnemyType::Boss:
            shape.setFillColor(sf::Color(255, 0, 100));
            break;
        default:
            shape.setFillColor(sf::Color::Red);
        }
        window.draw(shape);
    });

    // Player (flashing when invincible)
    bool visible = true;
    if (m_player.invincibilityTimer > 0.f) {
        int flash = static_cast<int>(m_player.invincibilityTimer / 0.1f);
        visible = (flash % 2 == 0);
    }
    if (visible) {
        sf::CircleShape shape(m_player.radius);
        shape.setPosition({m_player.pos.x - m_player.radius, m_player.pos.y - m_player.radius});
        shape.setFillColor(sf::Color::White);
        window.draw(shape);
    }
}

void PlayScene::drawUpgradeUI(sf::RenderWindow& window) {
    if (!m_font)
        return;

    sf::RectangleShape overlay({Config::VIEW_WIDTH, Config::VIEW_HEIGHT});
    overlay.setPosition({0.f, 0.f});
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    sf::Text title(*m_font);
    title.setString("Level Up!");
    title.setCharacterSize(28);
    title.setFillColor(sf::Color::Yellow);
    title.setPosition({320.f, 100.f});
    window.draw(title);

    int count = static_cast<int>(m_upgradeOptions.size());
    for (int i = 0; i < count; ++i) {
        const auto& opt = m_upgradeOptions[i];

        sf::Text txt(*m_font);
        txt.setCharacterSize(18);
        txt.setPosition({200.f, 180.f + static_cast<float>(i) * 60.f});

        if (i == m_selectedOption) {
            txt.setFillColor(sf::Color::Yellow);
            txt.setString("> " + std::string(opt.name) + "  [" + std::to_string(i + 1) + "]");
        } else {
            txt.setFillColor(sf::Color(200, 200, 200));
            txt.setString("  " + std::string(opt.name) + "  [" + std::to_string(i + 1) + "]");
        }
        window.draw(txt);

        sf::Text desc(*m_font);
        desc.setString(opt.description);
        desc.setCharacterSize(14);
        desc.setFillColor(sf::Color(160, 160, 160));
        desc.setPosition({220.f, 204.f + static_cast<float>(i) * 60.f});
        window.draw(desc);
    }

    sf::Text hint(*m_font);
    hint.setString("Arrow keys to select, Enter to confirm, or press 1-3");
    hint.setCharacterSize(13);
    hint.setFillColor(sf::Color(150, 150, 150));
    hint.setPosition({180.f, 480.f});
    window.draw(hint);
}
