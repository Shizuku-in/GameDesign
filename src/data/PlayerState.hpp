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
    float damageBonus = 0.f;       // 0=+0%, 1=+100%
    float cooldownReduction = 0.f; // 0=0%, 1=100%
    float xpMultiplier = 1.0f;

    /// 从角色定义初始化属性（覆盖默认值）。
    void initFromCharacter(float charHp, float charSpeed, float charRadius, float charArmor,
                           float charMagnet, float charDmgBonus, float charCDR) {
        hp = maxHp = charHp;
        speed = baseSpeed = charSpeed;
        radius = charRadius;
        armor = charArmor;
        magnetRange = charMagnet;
        damageBonus = charDmgBonus;
        cooldownReduction = charCDR;
    }

    int level = 1;
    float xp = 0.f;
    float xpToNext = Config::XP_BASE_THRESHOLD;

    // 角色精灵动画（由 PlayScene 赋值，WorldRenderer 读取）
    const SpriteSheet* spriteForward = nullptr; // 朝下
    const SpriteSheet* spriteBack = nullptr;    // 朝上
    const SpriteSheet* spriteSide = nullptr;    // 侧向移动（翻转实现左右朝向）
    const SpriteSheet* spriteIdle = nullptr;    // 待机（翻转实现左右）
    const SpriteSheet* spriteAttack = nullptr;  // 攻击（仅右朝向，翻转实现左朝向）
    const SpriteSheet* spriteHit = nullptr;     // 受击（仅右朝向，翻转实现左朝向）
    const SpriteSheet* spriteDeath = nullptr;   // 死亡（仅右朝向，翻转实现左朝向）
    // 移动中攻击/受击变体（仅 side 朝向通过翻转实现左右）
    const SpriteSheet* spriteMovingAttackForward = nullptr; // 移动中朝下开火
    const SpriteSheet* spriteMovingAttackBack = nullptr;    // 移动中朝上开火
    const SpriteSheet* spriteMovingAttackSide = nullptr;    // 移动中侧向开火
    const SpriteSheet* spriteMovingHitBack = nullptr;       // 移动中朝上受击
    const SpriteSheet* spriteMovingHitSide = nullptr;       // 移动中侧向受击
    const SpriteSheet* currentSprite = nullptr;             // 当前方向（每帧更新）
    int animFrame = 0;
    float animTimer = 0.f;

    // 角色朝向（仅 left/right，用于攻击/受击/待机的朝向选择）
    bool facingRight = true;

    // 动画状态计时器（> 0 表示对应动画播放中）
    float attackAnimTimer = 0.f;
    float hitAnimTimer = 0.f;
    float deathAnimTimer = 0.f;
};
