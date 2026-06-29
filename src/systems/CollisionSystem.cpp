#include "systems/CollisionSystem.hpp"
#include "math/Collision.hpp"
#include "data/Constants.hpp"
#include "audio/SoundPlayer.hpp"
#include <algorithm>
#include <vector>

namespace CollisionSystem {

namespace {
constexpr float CELL_SIZE = 100.f;
constexpr int GRID_COLS = static_cast<int>(Config::WORLD_WIDTH / CELL_SIZE) + 1;
constexpr int GRID_ROWS = static_cast<int>(Config::WORLD_HEIGHT / CELL_SIZE) + 1;

// 从 Constants 获取最大敌人半径（使用 C++14 constexpr std::max 支持）
constexpr float MAX_ENEMY_RADIUS = std::max({Config::ENEMY_RADIUS[0], Config::ENEMY_RADIUS[1],
                                             Config::ENEMY_RADIUS[2], Config::ENEMY_RADIUS[3]});

int getCellIndex(sf::Vector2f pos) {
    int cx = static_cast<int>(pos.x / CELL_SIZE);
    int cy = static_cast<int>(pos.y / CELL_SIZE);
    cx = std::max(0, std::min(cx, GRID_COLS - 1));
    cy = std::max(0, std::min(cy, GRID_ROWS - 1));
    return cy * GRID_COLS + cx;
}
} // namespace

void processCollisions(PlayerState& player, Pool<Enemy>& enemies, Pool<Projectile>& projectiles,
                       Pool<XPGem>& gems, int& score, SoundPlayer& sounds) {
    // 使用 thread_local 以保证线程安全，同时复用 std::vector 的 capacity
    // 避免如果直接放在栈上每帧触发 800+ 次析构和堆内存分配带来的极差性能
    thread_local std::vector<Enemy*> grid[GRID_COLS * GRID_ROWS];

    // 1. 构建空间哈希网格 (O(N))
    for (auto& cell : grid) {
        cell.clear(); // clear 只归零 size，保留 capacity，速度极快
    }
    enemies.forEach([&](Enemy& e) {
        if (e.hp > 0.f) {
            grid[getCellIndex(e.pos)].push_back(&e);
        }
    });

    // 2. 弹幕 vs 敌人 (利用网格进行宽阶段裁剪)
    projectiles.forEach([&](Projectile& p) {
        if (p.lifetime <= 0.f)
            return;

        // 计算弹幕可能触及的格子范围（考虑最大敌人半径，防止边缘漏判）
        float searchRadius = p.radius + MAX_ENEMY_RADIUS;
        int minCx = std::max(0, static_cast<int>((p.pos.x - searchRadius) / CELL_SIZE));
        int maxCx = std::min(GRID_COLS - 1, static_cast<int>((p.pos.x + searchRadius) / CELL_SIZE));
        int minCy = std::max(0, static_cast<int>((p.pos.y - searchRadius) / CELL_SIZE));
        int maxCy = std::min(GRID_ROWS - 1, static_cast<int>((p.pos.y + searchRadius) / CELL_SIZE));

        for (int cy = minCy; cy <= maxCy; ++cy) {
            for (int cx = minCx; cx <= maxCx; ++cx) {
                int cellIdx = cy * GRID_COLS + cx;
                for (Enemy* e : grid[cellIdx]) {
                    if (e->hp <= 0.f)
                        continue;

                    if (circleCircle(p.pos, p.radius, e->pos, e->radius)) {
                        e->hp -= p.damage;
                        --p.pierceCount;

                        if (e->hp <= 0.f) {
                            auto handle = gems.acquire();
                            auto* g = gems.get(handle);
                            if (g) {
                                g->pos = e->pos;
                                g->value = e->xpValue;
                                g->radius = 5.f;
                                g->magnetTimer = Config::XP_GEM_MAGNET_DELAY;
                            }
                            ++score;
                            sounds.kill();
                        } else {
                            sounds.hit();
                        }

                        if (p.pierceCount < 0) {
                            p.lifetime = 0.f;
                            goto NEXT_PROJECTILE; // 穿透耗尽，直接检测下一发弹幕
                        }
                    }
                }
            }
        }
    NEXT_PROJECTILE:;
    });

    // 3. 玩家 vs 敌人 (同样利用网格加速)
    if (player.invincibilityTimer <= 0.f) {
        bool tookDamage = false;

        float searchRadius = player.radius + MAX_ENEMY_RADIUS;
        int minCx = std::max(0, static_cast<int>((player.pos.x - searchRadius) / CELL_SIZE));
        int maxCx =
            std::min(GRID_COLS - 1, static_cast<int>((player.pos.x + searchRadius) / CELL_SIZE));
        int minCy = std::max(0, static_cast<int>((player.pos.y - searchRadius) / CELL_SIZE));
        int maxCy =
            std::min(GRID_ROWS - 1, static_cast<int>((player.pos.y + searchRadius) / CELL_SIZE));

        for (int cy = minCy; cy <= maxCy; ++cy) {
            for (int cx = minCx; cx <= maxCx; ++cx) {
                int cellIdx = cy * GRID_COLS + cx;
                for (Enemy* e : grid[cellIdx]) {
                    if (e->hp <= 0.f)
                        continue;

                    if (circleCircle(player.pos, player.radius, e->pos, e->radius)) {
                        float dmg = e->damage * Config::FIXED_DT * (1.f - player.armor);
                        player.hp -= dmg;
                        player.invincibilityTimer = Config::PLAYER_IFRAMES;
                        tookDamage = true;
                        goto PLAYER_HIT_DONE; // 无敌帧触发，跳出检测
                    }
                }
            }
        }
    PLAYER_HIT_DONE:
        if (tookDamage)
            sounds.hurt();
    }

    // 玩家 vs XP 宝石
    gems.forEach([&](XPGem& g) {
        if (circleCircle(player.pos, player.magnetRange, g.pos, g.radius)) {
            player.xp += g.value * player.xpMultiplier;
            g.value = -1.f;
            sounds.pickup();
        }
    });

    // 清理
    enemies.forEachHandle([&](Pool<Enemy>::Handle h, Enemy& e) {
        if (e.hp <= 0.f)
            enemies.release(h);
    });
    projectiles.forEachHandle([&](Pool<Projectile>::Handle h, Projectile& p) {
        if (p.lifetime <= 0.f)
            projectiles.release(h);
    });
    gems.forEachHandle([&](Pool<XPGem>::Handle h, XPGem& g) {
        if (g.value <= 0.f)
            gems.release(h);
    });
}

} // namespace CollisionSystem
