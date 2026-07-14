#include "core/Game.hpp"

#include "core/Scene.hpp"
#include "data/Constants.hpp"
#include "scenes/TitleScene.hpp"

#include <SFML/Window/VideoMode.hpp>

Game::Game()
    : m_window(sf::VideoMode({DEFAULT_WIDTH, DEFAULT_HEIGHT}), TITLE, sf::Style::Default,
               sf::State::Windowed) {
    m_window.setFramerateLimit(0); // 不限帧率 — 由游戏循环自行控制

    // 加载 UI 字体
    m_fonts.load("default", Config::FONT_DEFAULT_PATH);

    // 加载音效
    for (const auto& def : Config::SOUND_DEFS) {
        m_sounds.load(def.key, def.path);
    }

    m_scene = std::make_unique<TitleScene>(*this); // 直接赋值 — 构造时安全
}

Game::~Game() = default;

int Game::run() {
    m_clock.restart();
    sf::Time accumulator = sf::Time::Zero;

    while (m_window.isOpen() && m_running) {
        processEvents();

        sf::Time frameTime = m_clock.restart();

        ++m_fpsSampleFrames;
        m_fpsSampleTime += frameTime;
        if (m_fpsSampleTime >= sf::seconds(0.5f)) {
            m_framesPerSecond = static_cast<float>(m_fpsSampleFrames) / m_fpsSampleTime.asSeconds();
            m_fpsSampleTime = sf::Time::Zero;
            m_fpsSampleFrames = 0;
        }

        // 死亡螺旋保护
        if (frameTime > TIME_PER_FRAME_MAX) {
            frameTime = TIME_PER_FRAME_MAX;
        }

        accumulator += frameTime;

        // 固定时间步更新
        while (accumulator >= TIME_PER_FRAME) {
            update(TIME_PER_FRAME);
            accumulator -= TIME_PER_FRAME;
        }

        // 每次外层循环一次渲染
        render();

        // 延迟场景切换 — 此时场景代码不在调用栈上，安全
        if (m_pendingScene) {
            m_scene = std::move(m_pendingScene);
            m_pendingScene = nullptr;
        }
    }

    return 0;
}

void Game::changeScene(std::unique_ptr<Scene> scene) {
    // 延迟实际切换 — 在 update() / handleEvent() 中调用也安全
    m_pendingScene = std::move(scene);
}

sf::RenderWindow& Game::getWindow() { return m_window; }

ResourceManager<sf::Font>& Game::getFonts() { return m_fonts; }

ResourceManager<sf::SoundBuffer>& Game::getSounds() { return m_sounds; }

float Game::getFramesPerSecond() const { return m_framesPerSecond; }

void Game::processEvents() {
    while (const std::optional EVENT = m_window.pollEvent()) {
        if (EVENT->is<sf::Event::Closed>()) {
            m_window.close();
            m_running = false;
            return;
        }

        if (const auto* resized = EVENT->getIf<sf::Event::Resized>()) {
            handleWindowResize(*resized);
        }

        if (m_scene) {
            m_scene->handleEvent(*EVENT);
        }
    }
}

void Game::update(sf::Time dt) {
    if (m_scene) {
        m_scene->update(dt);
    }
}

void Game::render() {
    m_window.clear();

    if (m_scene) {
        m_scene->render(m_window);
    }

    m_window.display();
}

void Game::handleWindowResize(const sf::Event::Resized& /*resizeEvent*/) {
    // 保留默认视图为初始分辨率，不随窗口缩放
    // UI 坐标全部基于 VIEW_WIDTH/VIEW_HEIGHT，缩放会导致布局错乱。
    // 世界相机由 PlayScene 每帧显式设置，不受影响。
}
