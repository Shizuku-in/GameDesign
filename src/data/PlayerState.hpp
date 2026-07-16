#pragma once

#include "data/Constants.hpp"

#include <SFML/System/Vector2.hpp>

struct SpriteSheet;

/// 玩家状态 — 单例，不存入对象池。
struct PlayerState {
    /// 玩家在世界空间中的当前位置，由 PlayScene 初始化。
    sf::Vector2f pos;
    /// 已归一化的输入移动方向，每帧重新计算。
    sf::Vector2f vel;

    /// 当前实际移动速度（像素/秒）。
    float speed = 0.f;
    /// 未受升级影响的基础移动速度。
    float baseSpeed = 0.f;
    /// 当前生命值。
    float hp = 0.f;
    /// 生命值上限。
    float maxHp = 0.f;
    /// 与敌人发生圆形碰撞的半径。
    float radius = 0.f;

    /// 伤害减免比例，范围为 0 至 1。
    float armor = 0.f;
    /// 自动吸引经验宝石的范围。
    float magnetRange = 0.f;
    /// 武器伤害加成，1 表示额外 100%。
    float damageBonus = 0.f;
    /// 武器冷却缩减比例，1 表示完全取消冷却。
    float cooldownReduction = 0.f;
    /// 获得经验时应用的倍率。
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

    /// 当前玩家等级。
    int level = 1;
    /// 当前累计经验值。
    float xp = 0.f;
    /// 升至下一等级所需的经验值。
    float xpToNext = Config::XP_BASE_THRESHOLD;

    /// 朝下移动的精灵表。
    const SpriteSheet* spriteForward = nullptr;
    /// 朝上移动的精灵表。
    const SpriteSheet* spriteBack = nullptr;
    /// 侧向移动的精灵表，左右朝向通过翻转实现。
    const SpriteSheet* spriteSide = nullptr;
    /// 待机精灵表，左右朝向通过翻转实现。
    const SpriteSheet* spriteIdle = nullptr;
    /// 攻击精灵表，仅保留右朝向素材。
    const SpriteSheet* spriteAttack = nullptr;
    /// 受击精灵表，仅保留右朝向素材。
    const SpriteSheet* spriteHit = nullptr;
    /// 死亡精灵表，仅保留右朝向素材。
    const SpriteSheet* spriteDeath = nullptr;
    /// 移动中朝下攻击的精灵表。
    const SpriteSheet* spriteMovingAttackForward = nullptr;
    /// 移动中朝上攻击的精灵表。
    const SpriteSheet* spriteMovingAttackBack = nullptr;
    /// 移动中侧向攻击的精灵表。
    const SpriteSheet* spriteMovingAttackSide = nullptr;
    /// 移动中朝上受击的精灵表。
    const SpriteSheet* spriteMovingHitBack = nullptr;
    /// 移动中侧向受击的精灵表。
    const SpriteSheet* spriteMovingHitSide = nullptr;
    /// 当前帧渲染使用的精灵表。
    const SpriteSheet* currentSprite = nullptr;
    /// 当前精灵表的帧索引。
    int animFrame = 0;
    /// 距离切换到下一动画帧的累计时间。
    float animTimer = 0.f;

    /// 水平朝向，用于攻击、受击和待机精灵的翻转。
    bool facingRight = true;

    /// 攻击动作的剩余播放时间。
    float attackAnimTimer = 0.f;
    /// 受击动作的剩余播放时间。
    float hitAnimTimer = 0.f;
    /// 死亡动作的剩余播放时间。
    float deathAnimTimer = 0.f;
};
