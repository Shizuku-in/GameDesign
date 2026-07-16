#pragma once

#include <random>

/// 提供游戏共享的伪随机数引擎与常用分布。
class Random {
public:
    /// 使用指定种子初始化引擎，便于复现随机序列。
    static void init(unsigned int seed);
    /// 使用 random_device 提供的随机种子初始化引擎。
    static void init();

    /// 返回范围 [0.0, 1.0) 内的随机浮点数。
    static float getFloat();
    /// 返回范围 [min, max) 内的随机浮点数。
    static float getFloat(float min, float max);
    /// 返回范围 [min, max] 内的随机整数。
    static int getInt(int min, int max);

    /// 返回底层随机数引擎，供需要自定义分布的代码使用。
    static std::mt19937& getEngine();

private:
    /// 全局共享的 Mersenne Twister 伪随机数引擎。
    static std::mt19937 m_s_engine;
};
