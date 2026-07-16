#pragma once

#include "core/ResourceManager.hpp"
#include "data/Constants.hpp"

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

class Scene;

/// 管理窗口、主循环、资源缓存与当前场景的游戏运行时。
class Game {
public:
    /// 创建窗口、加载全局资源并进入标题场景。
    Game();
    /// 销毁场景与运行时资源。
    ~Game();

    /// 运行主循环，直到窗口关闭；返回进程退出码。
    int run();

    /// 请求在当前帧末尾切换到指定场景。
    void changeScene(std::unique_ptr<Scene> scene);
    /// 返回游戏窗口。
    sf::RenderWindow& getWindow();
    /// 返回字体资源缓存。
    ResourceManager<sf::Font>& getFonts();
    /// 返回音效资源缓存。
    ResourceManager<sf::SoundBuffer>& getSounds();
    /// 返回最近半秒采样得到的渲染帧率。
    float getFramesPerSecond() const;

private:
    /// 分发窗口事件给当前场景。
    void processEvents();
    /// 用固定步长更新当前场景。
    void update(sf::Time dt);
    /// 绘制并交换当前帧。
    void render();
    /// 处理窗口尺寸变化，保持逻辑视口不变。
    void handleWindowResize(const sf::Event::Resized& resizeEvent);

    /// 窗口和默认视图的逻辑宽度。
    static constexpr unsigned int DEFAULT_WIDTH = static_cast<unsigned int>(Config::VIEW_WIDTH);
    /// 窗口和默认视图的逻辑高度。
    static constexpr unsigned int DEFAULT_HEIGHT = static_cast<unsigned int>(Config::VIEW_HEIGHT);
    /// 窗口标题。
    static constexpr const char* TITLE = "SFML 3.1 Game";

    /// 固定更新步长，每秒执行 60 次逻辑更新。
    static constexpr sf::Time TIME_PER_FRAME = sf::seconds(1.0f / 60.0f);
    /// 单帧允许累积的最大时间，避免低帧率下的死亡螺旋。
    static constexpr sf::Time TIME_PER_FRAME_MAX = sf::seconds(1.0f / 15.0f);

    /// 用于事件处理和绘制的主窗口。
    sf::RenderWindow m_window;
    /// 当前正在运行的场景。
    std::unique_ptr<Scene> m_scene;
    /// 待在帧末应用的场景，保证更新期间切换场景安全。
    std::unique_ptr<Scene> m_pendingScene;
    /// 共享字体资源缓存。
    ResourceManager<sf::Font> m_fonts;
    /// 共享音效缓冲资源缓存。
    ResourceManager<sf::SoundBuffer> m_sounds;
    /// 测量外层渲染帧耗时的时钟。
    sf::Clock m_clock;

    /// 最近一次 FPS 采样结果。
    float m_framesPerSecond = 0.f;
    /// 当前 FPS 采样窗口累计的真实时间。
    sf::Time m_fpsSampleTime = sf::Time::Zero;
    /// 当前 FPS 采样窗口内完成的渲染帧数。
    unsigned int m_fpsSampleFrames = 0;

    /// 主循环是否继续执行。
    bool m_running = true;
};
