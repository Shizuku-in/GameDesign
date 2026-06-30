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
        .spriteForward = "assets/sprites/character/elf/movement/forward.png",
        .spriteBack = "assets/sprites/character/elf/movement/back.png",
        .spriteLeft = "assets/sprites/character/elf/movement/left.png",
        .spriteRight = "assets/sprites/character/elf/movement/right.png",
        .spriteIdle = "assets/sprites/character/elf/idle/idle.png",
    },
};

static_assert(sizeof(CHARACTER_DEFS) / sizeof(CHARACTER_DEFS[0]) ==
                  static_cast<int>(CharacterType::Count),
              "CHARACTER_DEFS must have one entry per CharacterType");
