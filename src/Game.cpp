#include "Game.hpp"
#include "GameScene.hpp"
#include "Scene.hpp"

#include <SFML/Window/VideoMode.hpp>

Game::Game()
    : m_window(sf::VideoMode({DEFAULT_WIDTH, DEFAULT_HEIGHT}),
               TITLE,
               sf::Style::Default,
               sf::State::Windowed)
{
    m_window.setFramerateLimit(0); // uncapped — we control timing ourselves
    changeScene(std::make_unique<GameScene>(*this));
}

Game::~Game() = default;

int Game::run()
{
    m_clock.restart();
    sf::Time accumulator = sf::Time::Zero;

    while (m_window.isOpen() && m_running) {
        processEvents();

        sf::Time frameTime = m_clock.restart();

        // Spiral-of-death guard
        if (frameTime > TIME_PER_FRAME_MAX)
            frameTime = TIME_PER_FRAME_MAX;

        accumulator += frameTime;

        // Fixed-timestep updates
        while (accumulator >= TIME_PER_FRAME) {
            update(TIME_PER_FRAME);
            accumulator -= TIME_PER_FRAME;
        }

        // One render pass per outer-loop iteration
        render();
    }

    return 0;
}

void Game::changeScene(std::unique_ptr<Scene> scene)
{
    m_scene = std::move(scene);
}

sf::RenderWindow& Game::getWindow()
{
    return m_window;
}

void Game::processEvents()
{
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            m_running = false;
            return;
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>())
            handleWindowResize(*resized);

        if (m_scene)
            m_scene->handleEvent(*event);
    }
}

void Game::update(sf::Time dt)
{
    if (m_scene)
        m_scene->update(dt);
}

void Game::render()
{
    m_window.clear();

    if (m_scene)
        m_scene->render(m_window);

    m_window.display();
}

void Game::handleWindowResize(const sf::Event::Resized& resizeEvent)
{
    // Keep the view stretched to match the new window size
    m_window.setView(sf::View(sf::FloatRect(
        {0.f, 0.f},
        static_cast<sf::Vector2f>(resizeEvent.size))));
}
