#include "gameplay/EnemyDefs.hpp"

const EnemyDef ENEMY_DEFS[] = {
    // Basic — 骷髅战士
    {EnemyType::Basic, "Skeleton", 20.f, 80.f, 10.f, 14.f, 2.5f, 1.f, 1.0f, 0.f,
     "assets/sprites/enemies/enemies-skeleton1_movement.png",
     "assets/sprites/enemies/enemies-skeleton1_take_damage.png"},

    // Fast — 骷髅疾行者
    {EnemyType::Fast, "Skeleton Runner", 10.f, 160.f, 8.f, 10.f, 2.5f, 2.f, 0.7f, 30.f,
     "assets/sprites/enemies/enemies-skeleton2_movement.png",
     "assets/sprites/enemies/enemies-skeleton2_take_damage.png"},

    // Tank — 吸血鬼
    {EnemyType::Tank, "Vampire", 80.f, 50.f, 20.f, 22.f, 2.5f, 5.f, 0.4f, 60.f,
     "assets/sprites/enemies/enemies-vampire_movement.png",
     "assets/sprites/enemies/enemies-vampire_take_damage.png"},

    // Boss — 吸血鬼领主
    {EnemyType::Boss, "Vampire Lord", 300.f, 60.f, 30.f, 32.f, 3.0f, 50.f, 0.f, -1.f,
     "assets/sprites/enemies/enemies-vampire_movement.png",
     "assets/sprites/enemies/enemies-vampire_take_damage.png"},
};

static_assert(sizeof(ENEMY_DEFS) / sizeof(ENEMY_DEFS[0]) == static_cast<int>(EnemyType::Count),
              "ENEMY_DEFS must have one entry per EnemyType");
