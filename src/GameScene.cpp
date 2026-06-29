#include "GameScene.hpp"
#include "Game.hpp"

#include <SFML/Window/Keyboard.hpp>

#include <cmath>

GameScene::GameScene(Game& game)
    : m_game(game)
    , m_player(PLAYER_RADIUS)
    , m_velocity({0.f, 0.f})
    , m_bgColor(sf::Color::Black)
{
    m_player.setFillColor(sf::Color::Cyan);
    m_player.setOrigin({PLAYER_RADIUS, PLAYER_RADIUS});

    // Start centered in the window
    sf::Vector2u size = m_game.getWindow().getSize();
    m_player.setPosition({
        static_cast<float>(size.x) / 2.f,
        static_cast<float>(size.y) / 2.f
    });
}

void GameScene::handleEvent(const sf::Event& event)
{
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape)
            m_game.getWindow().close();
    }
}

void GameScene::update(sf::Time dt)
{
    float dtSec = dt.asSeconds();

    // --- Input ---
    m_velocity = {0.f, 0.f};

    using Key = sf::Keyboard::Key;
    auto isDown = [](Key k) { return sf::Keyboard::isKeyPressed(k); };

    if (isDown(Key::Right) || isDown(Key::D)) m_velocity.x += 1.f;
    if (isDown(Key::Left)  || isDown(Key::A)) m_velocity.x -= 1.f;
    if (isDown(Key::Down)  || isDown(Key::S)) m_velocity.y += 1.f;
    if (isDown(Key::Up)    || isDown(Key::W)) m_velocity.y -= 1.f;

    // Normalize diagonal movement
    float len = std::sqrt(m_velocity.x * m_velocity.x +
                          m_velocity.y * m_velocity.y);
    if (len > 0.f) {
        m_velocity.x /= len;
        m_velocity.y /= len;
    }

    // --- Move ---
    m_player.move(m_velocity * PLAYER_SPEED * dtSec);

    // --- Collide with window edges ---
    checkBounds(m_game.getWindow().getSize());

    // --- Cycle background hue ---
    m_bgHue += BG_HUE_SPEED * dtSec;
    if (m_bgHue >= 360.f) m_bgHue -= 360.f;

    // HSV-to-RGB for a dark, low-saturation background
    float s = 0.3f, v = 0.15f;
    float c = v * s;
    float x = c * (1.f - std::fabs(std::fmod(m_bgHue / 60.f, 2.f) - 1.f));
    float m = v - c;
    float rp = 0.f, gp = 0.f, bp = 0.f;

    if      (m_bgHue <  60.f) { rp = c; gp = x; bp = 0; }
    else if (m_bgHue < 120.f) { rp = x; gp = c; bp = 0; }
    else if (m_bgHue < 180.f) { rp = 0; gp = c; bp = x; }
    else if (m_bgHue < 240.f) { rp = 0; gp = x; bp = c; }
    else if (m_bgHue < 300.f) { rp = x; gp = 0; bp = c; }
    else                      { rp = c; gp = 0; bp = x; }

    m_bgColor = sf::Color(
        static_cast<std::uint8_t>((rp + m) * 255.f),
        static_cast<std::uint8_t>((gp + m) * 255.f),
        static_cast<std::uint8_t>((bp + m) * 255.f)
    );
}

void GameScene::render(sf::RenderWindow& window)
{
    window.clear(m_bgColor);
    window.draw(m_player);
}

void GameScene::checkBounds(const sf::Vector2u& windowSize)
{
    sf::Vector2f pos = m_player.getPosition();
    float r = PLAYER_RADIUS;
    float w = static_cast<float>(windowSize.x);
    float h = static_cast<float>(windowSize.y);

    if (pos.x - r < 0.f)  { pos.x = r;    m_velocity.x *= -1.f; }
    if (pos.x + r > w)    { pos.x = w - r; m_velocity.x *= -1.f; }
    if (pos.y - r < 0.f)  { pos.y = r;    m_velocity.y *= -1.f; }
    if (pos.y + r > h)    { pos.y = h - r; m_velocity.y *= -1.f; }

    m_player.setPosition(pos);
}
