#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"

class SoundPlayer;

/// 碰撞检测 + 死实体清理。无状态，纯函数。
namespace CollisionSystem {

/// 执行全部碰撞检测并清理死实体。
/// 修改 player (hp/xp)、三个池、score，在命中/击杀/拾取/受伤时播放音效。
void processCollisions(PlayerState& player, Pool<Enemy>& enemies, Pool<Projectile>& projectiles,
                       Pool<XPGem>& gems, int& score, SoundPlayer& sounds);

} // namespace CollisionSystem
