#include "systems/CollisionSystem.hpp"
#include "data/Collision.hpp"
#include "data/Constants.hpp"
#include "systems/SoundPlayer.hpp"

namespace CollisionSystem {

void processCollisions(PlayerState& player, Pool<Enemy>& enemies, Pool<Projectile>& projectiles,
                       Pool<XPGem>& gems, int& score, SoundPlayer& sounds) {
    // 弹幕 vs 敌人
    projectiles.forEach([&](Projectile& p) {
        enemies.forEach([&](Enemy& e) {
            if (e.hp <= 0.f)
                return;
            if (circleCircle(p.pos, p.radius, e.pos, e.radius)) {
                e.hp -= p.damage;
                --p.pierceCount;
                if (p.pierceCount < 0)
                    p.lifetime = 0.f;
                if (e.hp <= 0.f) {
                    auto handle = gems.acquire();
                    auto* g = gems.get(handle);
                    if (g) {
                        g->pos = e.pos;
                        g->value = e.xpValue;
                        g->radius = 5.f;
                        g->magnetTimer = Config::XP_GEM_MAGNET_DELAY;
                    }
                    ++score;
                    sounds.kill();
                } else {
                    sounds.hit();
                }
            }
        });
    });

    // 玩家 vs 敌人
    if (player.invincibilityTimer <= 0.f) {
        bool tookDamage = false;
        enemies.forEach([&](Enemy& e) {
            if (e.hp <= 0.f)
                return;
            if (circleCircle(player.pos, player.radius, e.pos, e.radius)) {
                float dmg = e.damage * Config::FIXED_DT * (1.f - player.armor);
                player.hp -= dmg;
                player.invincibilityTimer = Config::PLAYER_IFRAMES;
                tookDamage = true;
            }
        });
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
