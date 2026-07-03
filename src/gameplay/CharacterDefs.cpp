#include "gameplay/CharacterDefs.hpp"

const CharacterDef CHARACTER_DEFS[] = {
    // Elf — 精灵射手：脆皮高速，拾取范围大
    {
        .type = CharacterType::Elf,
        .name = "Elf",
        .frameWidth = 48,
        .frameHeight = 48,
        .hp = 10.f,
        .speed = 240.f,
        .radius = 16.f,
        .armor = 0.f,
        .magnetRange = 100.f,
        .damageBonus = 0.f,
        .cooldownReduction = 0.f,
        .spriteForward = "assets/sprites/character/elf/movement_forward.png",
        .spriteBack = "assets/sprites/character/elf/movement_back.png",
        .spriteSide = "assets/sprites/character/elf/movement_side.png",
        .spriteIdle = "assets/sprites/character/elf/idle.png",
        .spriteAttack = "assets/sprites/character/elf/attack.png",
        .spriteHit = "assets/sprites/character/elf/hit.png",
        .spriteDeath = "assets/sprites/character/elf/death.png",
        .spriteMovingAttackForward = "assets/sprites/character/elf/moving_attack_forward.png",
        .spriteMovingAttackBack = "assets/sprites/character/elf/moving_attack_back.png",
        .spriteMovingAttackSide = "assets/sprites/character/elf/moving_attack_side.png",
        .spriteMovingHitBack = "assets/sprites/character/elf/moving_hit_back.png",
        .spriteMovingHitSide = "assets/sprites/character/elf/moving_hit_side.png",
    },
};

static_assert(sizeof(CHARACTER_DEFS) / sizeof(CHARACTER_DEFS[0]) ==
                  static_cast<int>(CharacterType::Count),
              "CHARACTER_DEFS must have one entry per CharacterType");
