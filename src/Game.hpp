#pragma once

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

private:
    void processEvents();
    void update(sf::Time dt);
    void render();
    void handleWindowResize(const sf::Event::Resized& resizeEvent);

    static constexpr unsigned int DEFAULT_WIDTH = 800;
    static constexpr unsigned int DEFAULT_HEIGHT = 600;
    static constexpr const char* TITLE = "SFML 3.1 Game";

    // Fixed timestep: 60 updates per second
    static constexpr sf::Time TIME_PER_FRAME = sf::seconds(1.0f / 60.0f);
    // Spiral-of-death cap: at most 4 updates per render
    static constexpr sf::Time TIME_PER_FRAME_MAX = sf::seconds(1.0f / 15.0f);

    sf::RenderWindow m_window;
    std::unique_ptr<Scene> m_scene;
    sf::Clock m_clock;

    bool m_running = true;
};
