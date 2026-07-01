#pragma once

#include <SFML/System/Vector2.hpp>

/// 圆-圆碰撞检测（避免开方，只比较平方距离）。
constexpr bool circleCircle(sf::Vector2f p1, float r1, sf::Vector2f p2, float r2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float r = r1 + r2;
    return dx * dx + dy * dy <= r * r;
}

/// 两点之间的平方欧氏距离。
constexpr float distanceSq(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return (dx * dx) + (dy * dy);
}
