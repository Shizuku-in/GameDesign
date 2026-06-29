#pragma once

#include "core/ResourceManager.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

class Scene;

class Game {
public:
    Game();
    ~Game();

    int run();

    void changeScene(std::unique_ptr<Scene> scene);
    sf::RenderWindow& getWindow();
    ResourceManager<sf::Font>& getFonts();

private:
    void processEvents();
    void update(sf::Time dt);
    void render();
    void handleWindowResize(const sf::Event::Resized& resizeEvent);

    static constexpr unsigned int DEFAULT_WIDTH = 1920;
    static constexpr unsigned int DEFAULT_HEIGHT = 1080;
    static constexpr const char* TITLE = "SFML 3.1 Game";

    // 固定时间步：每秒 60 次更新
    static constexpr sf::Time TIME_PER_FRAME = sf::seconds(1.0f / 60.0f);
    // 死亡螺旋保护：每次渲染最多执行 4 次更新
    static constexpr sf::Time TIME_PER_FRAME_MAX = sf::seconds(1.0f / 15.0f);

    sf::RenderWindow m_window;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Scene> m_pendingScene; // 延迟场景切换（在 update() 中调用也安全）
    ResourceManager<sf::Font> m_fonts;
    sf::Clock m_clock;

    bool m_running = true;
};
