#pragma once

#include <SFML/Graphics/Texture.hpp>

/// 水平条带精灵表。每帧等宽等高，从左到右水平排列。
struct SpriteSheet {
    sf::Texture texture;
    int frameWidth = 0;  // 单帧宽度（像素）
    int frameHeight = 0; // 单帧高度（像素）
    int frameCount = 0;  // 总帧数

    /// 从文件加载精灵表并自动计算帧数。
    /// frameSize = 单帧宽高，纹理宽度 / frameSize 即为帧数。
    bool loadFromFile(const char* path, int frameW, int frameH) {
        if (frameW <= 0) {
            return false;
        }
        if (!texture.loadFromFile(path)) {
            return false;
        }
        frameWidth = frameW;
        frameHeight = frameH;
        frameCount = static_cast<int>(texture.getSize().x) / frameW;
        return frameCount > 0;
    }
};
