#include "gameplay/CharacterDefs.hpp"

const CharacterDef CHARACTER_DEFS[] = {
    // Elf — 精灵射手：脆皮高速，拾取范围大
    {CharacterType::Elf, "Elf", 48, 48,
     80.f,  // hp
     240.f, // speed
     16.f,  // radius
     0.f,   // armor
     100.f, // magnetRange
     "assets/sprites/character/elf/movement/forward.png",
     "assets/sprites/character/elf/movement/back.png",
     "assets/sprites/character/elf/movement/left.png",
     "assets/sprites/character/elf/movement/right.png",
     "assets/sprites/character/elf/idle/idle.png"},
};

static_assert(sizeof(CHARACTER_DEFS) / sizeof(CHARACTER_DEFS[0]) ==
                  static_cast<int>(CharacterType::Count),
              "CHARACTER_DEFS must have one entry per CharacterType");
