#include "gameplay/CharacterDefs.hpp"

const CharacterDef CHARACTER_DEFS[] = {
    // Elf — 精灵射手
    {CharacterType::Elf, "Elf", 48, 48, "assets/sprites/character/elf/movement/forward.png",
     "assets/sprites/character/elf/movement/back.png",
     "assets/sprites/character/elf/movement/left.png",
     "assets/sprites/character/elf/movement/right.png",
     "assets/sprites/character/elf/idle/idle.png"},
};

static_assert(sizeof(CHARACTER_DEFS) / sizeof(CHARACTER_DEFS[0]) ==
                  static_cast<int>(CharacterType::Count),
              "CHARACTER_DEFS must have one entry per CharacterType");
