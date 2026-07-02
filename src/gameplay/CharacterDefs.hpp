#pragma once

#include <cstdint>

/// 角色类型枚举——按添加顺序排列。
enum class CharacterType : std::uint8_t { Elf, Count };

/// 角色定义——精灵路径 + 帧尺寸 + 属性，集中管理。
struct CharacterDef {
    CharacterType type;
    const char* name;
    int frameWidth;
    int frameHeight;

    // 属性
    float hp;                // 最大生命值
    float speed;             // 移动速度（像素/秒）
    float radius;            // 碰撞半径
    float armor;             // 伤害减免 0–1
    float magnetRange;       // 宝石拾取范围
    float damageBonus;       // 伤害加成 0=+0% 1=+100%
    float cooldownReduction; // 冷却缩减 0=0% 1=100%

    // 各方向精灵表路径
    const char* spriteForward; // 面对镜头（朝下）
    const char* spriteBack;    // 背对镜头（朝上）
    const char* spriteSide;    // 侧向移动
    const char* spriteIdle;    // 待机

    // 以下精灵仅保留右朝向，左朝向通过翻转实现
    const char* spriteAttack; // 攻击
    const char* spriteHit;    // 受击
    const char* spriteDeath;  // 死亡
};

/// 所有角色定义表，按 CharacterType 枚举顺序排列。
extern const CharacterDef CHARACTER_DEFS[];
