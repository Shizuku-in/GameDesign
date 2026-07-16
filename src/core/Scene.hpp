#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

namespace sf {
class RenderWindow;
}

/// 场景的抽象接口，由游戏循环驱动事件、更新与渲染。
class Scene {
public:
    /// 支持通过基类指针安全销毁派生场景。
    virtual ~Scene() = default;

    /// 接收窗口事件；派生类可按需重写。
    virtual void handleEvent(const sf::Event& /*unused*/) {}
    /// 推进场景逻辑一个固定时间步。
    virtual void update(sf::Time dt) = 0;
    /// 向窗口绘制当前场景。
    virtual void render(sf::RenderWindow& window) = 0;
};
