#include "core/Random.hpp"

std::mt19937 Random::m_s_engine;

void Random::init(unsigned int seed) { m_s_engine.seed(seed); }

void Random::init() {
    std::random_device rd;
    init(rd());
}

float Random::getFloat() {
    static std::uniform_real_distribution<float> dist(0.f, 1.f);
    return dist(m_s_engine);
}

float Random::getFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_s_engine);
}

int Random::getInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_s_engine);
}

std::mt19937& Random::getEngine() { return m_s_engine; }
