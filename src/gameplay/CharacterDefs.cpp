#include "gameplay/CharacterDefs.hpp"

const CharacterDef CHARACTER_DEFS[] = {
    // Elf — 精灵射手：脆皮高速，拾取范围大
    {
        .type = CharacterType::Elf,
        .name = "Elf",
        .frameWidth = 48,
        .frameHeight = 48,
        .hp = 80.f,
        .speed = 240.f,
        .radius = 16.f,
        .armor = 0.f,
        .magnetRange = 100.f,
        .damageBonus = 0.f,
        .cooldownReduction = 0.f,
        .spriteForward = "assets/sprites/character/elf/movement/forward.png",
        .spriteBack = "assets/sprites/character/elf/movement/back.png",
        .spriteLeft = "assets/sprites/character/elf/movement/left.png",
        .spriteRight = "assets/sprites/character/elf/movement/right.png",
        .spriteIdle = nullptr, // 弃用全方向 idle，改用 idle_left/idle_right
        .spriteIdleLeft = "assets/sprites/character/elf/idle/idle_left.png",
        .spriteIdleRight = "assets/sprites/character/elf/idle/idle_right.png",
        .spriteAttackLeft = "assets/sprites/character/elf/attack/attack_left.png",
        .spriteAttackRight = "assets/sprites/character/elf/attack/attack_right.png",
        .spriteHitLeft = "assets/sprites/character/elf/hit/hit_left.png",
        .spriteHitRight = "assets/sprites/character/elf/hit/hit_right.png",
        .spriteDeathLeft = "assets/sprites/character/elf/death/death_left.png",
        .spriteDeathRight = "assets/sprites/character/elf/death/death_right.png",
    },
};

static_assert(sizeof(CHARACTER_DEFS) / sizeof(CHARACTER_DEFS[0]) ==
                  static_cast<int>(CharacterType::Count),
              "CHARACTER_DEFS must have one entry per CharacterType");
