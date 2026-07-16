#pragma once

#include "core/Scene.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <optional>

class Game;

/// 游戏启动后的标题场景，负责显示 Logo 并接收开始/退出输入。
class TitleScene : public Scene {
public:
    /// 使用游戏资源创建标题界面。
    explicit TitleScene(Game& game);

    /// 处理开始游戏和退出游戏的按键事件。
    void handleEvent(const sf::Event& event) override;
    /// 推进标题文字的呼吸动画。
    void update(sf::Time dt) override;
    /// 绘制 Logo、标题和操作提示。
    void render(sf::RenderWindow& window) override;

private:
    /// 用于场景切换和获取全局资源的游戏实例。
    Game& m_game;
    /// 默认 UI 字体；资源加载失败时为空。
    const sf::Font* m_font = nullptr;

    /// Logo 原始纹理。
    sf::Texture m_logoTexture;
    /// 加载 Logo 成功时创建的精灵。
    std::optional<sf::Sprite> m_logoSprite;
    /// 开始游戏提示文本。
    sf::Text m_startText;
    /// 控制说明文本。
    sf::Text m_controlsText;

    /// 用于开始提示呼吸效果的累计时间。
    float m_elapsed = 0.f;
};
