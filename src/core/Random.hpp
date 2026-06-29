#pragma once

#include <random>

class Random {
public:
    static void init(unsigned int seed);
    static void init(); // 自动使用 random_device 播种

    static float getFloat();                     // [0.0, 1.0)
    static float getFloat(float min, float max); // [min, max)
    static int getInt(int min, int max);         // [min, max]

    static std::mt19937& getEngine();

private:
    static std::mt19937 s_engine;
};
