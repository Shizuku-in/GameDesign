#include "gameplay/EnemyDefs.hpp"

const EnemyDef ENEMY_DEFS[] = {
    // Basic — 骷髅战士
    {
        .type = EnemyType::Basic,
        .name = "Skeleton",
        .hp = 20.f,
        .speed = 80.f,
        .damage = 10.f,
        .radius = 14.f,
        .spriteScale = 2.5f,
        .xpValue = 1.f,
        .spawnWeight = 1.0f,
        .appearTime = 0.f,
        .spriteMovePath = "assets/sprites/enemies/enemies-skeleton1_movement.png",
        .spriteDamagedPath = "assets/sprites/enemies/enemies-skeleton1_take_damage.png",
    },

    // Fast — 骷髅疾行者
    {
        .type = EnemyType::Fast,
        .name = "Skeleton Runner",
        .hp = 10.f,
        .speed = 160.f,
        .damage = 8.f,
        .radius = 10.f,
        .spriteScale = 2.5f,
        .xpValue = 2.f,
        .spawnWeight = 0.7f,
        .appearTime = 30.f,
        .spriteMovePath = "assets/sprites/enemies/enemies-skeleton2_movement.png",
        .spriteDamagedPath = "assets/sprites/enemies/enemies-skeleton2_take_damage.png",
    },

    // Tank — 吸血鬼
    {
        .type = EnemyType::Tank,
        .name = "Vampire",
        .hp = 80.f,
        .speed = 50.f,
        .damage = 20.f,
        .radius = 22.f,
        .spriteScale = 2.5f,
        .xpValue = 5.f,
        .spawnWeight = 0.4f,
        .appearTime = 60.f,
        .spriteMovePath = "assets/sprites/enemies/enemies-vampire_movement.png",
        .spriteDamagedPath = "assets/sprites/enemies/enemies-vampire_take_damage.png",
    },

    // Boss — 吸血鬼领主
    {
        .type = EnemyType::Boss,
        .name = "Vampire Lord",
        .hp = 300.f,
        .speed = 60.f,
        .damage = 30.f,
        .radius = 32.f,
        .spriteScale = 3.0f,
        .xpValue = 50.f,
        .spawnWeight = 0.f,
        .appearTime = -1.f,
        .spriteMovePath = "assets/sprites/enemies/enemies-vampire_movement.png",
        .spriteDamagedPath = "assets/sprites/enemies/enemies-vampire_take_damage.png",
    },
};

static_assert(sizeof(ENEMY_DEFS) / sizeof(ENEMY_DEFS[0]) == static_cast<int>(EnemyType::Count),
              "ENEMY_DEFS must have one entry per EnemyType");
