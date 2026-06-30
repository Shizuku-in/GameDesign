#pragma once

#include "data/Constants.hpp"

#include <SFML/System/Vector2.hpp>

struct SpriteSheet;

/// 玩家状态 — 单例，不存入对象池。
struct PlayerState {
    sf::Vector2f pos; // 由 PlayScene 根据地图设置初始位置
    sf::Vector2f vel; // 输入方向（已归一化），每帧计算

    float speed = 0.f;
    float baseSpeed = 0.f;
    float hp = 0.f;
    float maxHp = 0.f;
    float radius = 0.f;

    float armor = 0.f;
    float magnetRange = 0.f;
    float xpMultiplier = 1.0f;

    /// 从角色定义初始化属性（覆盖默认值）。
    void initFromCharacter(float charHp, float charSpeed, float charRadius, float charArmor,
                           float charMagnet) {
        hp = maxHp = charHp;
        speed = baseSpeed = charSpeed;
        radius = charRadius;
        armor = charArmor;
        magnetRange = charMagnet;
    }

    int level = 1;
    float xp = 0.f;
    float xpToNext = Config::XP_BASE_THRESHOLD;

    float invincibilityTimer = 0.f; // > 0 表示无敌，不受伤害

    // 角色精灵动画（由 PlayScene 赋值，WorldRenderer 读取）
    const SpriteSheet* spriteForward = nullptr; // 朝下
    const SpriteSheet* spriteBack = nullptr;    // 朝上
    const SpriteSheet* spriteLeft = nullptr;
    const SpriteSheet* spriteRight = nullptr;
    const SpriteSheet* spriteIdle = nullptr;    // 待机
    const SpriteSheet* currentSprite = nullptr; // 当前方向（每帧更新）
    int animFrame = 0;
    float animTimer = 0.f;
};
