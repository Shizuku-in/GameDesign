#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <cstdint>
#include <vector>

/// Tiled TMX 地图渲染器。加载一次，每帧绘制。
class TilemapRenderer {
public:
    /// 从 TMX 文件加载地图并构建顶点。
    bool loadFromFile(const char* tmxPath, const char* tilesetPath);

    /// 绘制地面层。
    void draw(sf::RenderWindow& window);

    /// 地图尺寸（像素）。
    float getWidth() const { return static_cast<float>(m_mapWidth * m_tileWidth); }
    float getHeight() const { return static_cast<float>(m_mapHeight * m_tileHeight); }

private:
    sf::Texture m_tileset;
    sf::VertexArray m_vertices{sf::PrimitiveType::Triangles};
    int m_tileWidth = 0;
    int m_tileHeight = 0;
    int m_mapWidth = 0; // 瓦片数
    int m_mapHeight = 0;
    int m_tilesetCols = 0;
    int m_tilesetRows = 0;
    std::uint32_t m_firstGid = 1;

    void buildVertices(const std::vector<std::uint32_t>& tiles);
};
