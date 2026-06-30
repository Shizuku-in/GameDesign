#pragma once

#include "data/EntityTypes.hpp" // EnemyType

/// 敌人类型定义——所有属性集中在一处。
struct EnemyDef {
    EnemyType type;
    const char* name; // 显示名
    float hp;
    float speed;
    float damage;      // 接触伤害（每秒）
    float radius;      // 碰撞半径
    float spriteScale; // 精灵绘制缩放
    float xpValue;     // 击杀掉落 XP
    float spawnWeight; // 随机生成权重（0 = 不参与随机，如 Boss）
    float appearTime;  // 首次出现时间（秒），-1 = 不按权重生成

    // 精灵表
    const char* spriteMovePath;
    const char* spriteDamagedPath;
    int frameWidth = 32;
    int frameHeight = 32;
};

/// 所有敌人类型的定义表，按 EnemyType 枚举顺序排列。
extern const EnemyDef ENEMY_DEFS[];
