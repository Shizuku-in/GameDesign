#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace tmx {
class Layer;
class LayerGroup;
class Map;
} // namespace tmx

/// 对象形状类型，与 tmxlite::Object::Shape 一一对应。
enum class ObjectShape : int { Rectangle = 0, Ellipse, Point, Polygon, Polyline, Text };

/// 对象层解析结果，供碰撞 / 生成等系统读取。
struct MapObject {
    std::string name;
    std::string type;
    float x = 0.f;
    float y = 0.f;
    float width = 0.f;
    float height = 0.f;
    float rotation = 0.f;
    ObjectShape shape = ObjectShape::Rectangle;
    std::vector<sf::Vector2f> points;
};

/// Tiled TMX 地图渲染器。加载一次，每帧绘制。
class TilemapRenderer {
public:
    /// 从 TMX 文件加载地图并构建顶点。tileset 纹理路径由 TMX 内部解析。
    bool loadFromFile(const char* tmxPath);

    /// 绘制所有可见 tile 层。
    void draw(sf::RenderWindow& window);

    /// 地图尺寸（像素）。
    [[nodiscard]] float getWidth() const { return static_cast<float>(m_mapWidth * m_tileWidth); }
    [[nodiscard]] float getHeight() const { return static_cast<float>(m_mapHeight * m_tileHeight); }

    /// 对象层数据（只读）。
    [[nodiscard]] const std::vector<MapObject>& getObjects() const { return m_objects; }

private:
    /// 单个 tileset 纹理及其元数据。
    struct TilesetTexture {
        sf::Texture texture;
        std::uint32_t firstGid = 0;
        std::uint32_t lastGid = 0;
        int columns = 0;
        int rows = 0;
        int tileWidth = 0;
        int tileHeight = 0;
        int margin = 0;
        int spacing = 0;
    };

    /// 一次绘制单元：一个 VertexArray 绑一个纹理，不透明度已烘焙到顶点色。
    struct RenderBatch {
        sf::VertexArray vertices{sf::PrimitiveType::Triangles};
        const sf::Texture* texture = nullptr;
    };

    /// 展平后的层条目（加载阶段临时使用）。
    struct FlatEntry {
        const tmx::Layer* layer = nullptr;
        float opacity = 1.f;
        bool visible = true;
    };

    std::vector<TilesetTexture> m_tilesets;
    std::vector<RenderBatch> m_batches;
    std::vector<MapObject> m_objects;

    int m_tileWidth = 0;
    int m_tileHeight = 0;
    int m_mapWidth = 0; // 瓦片数
    int m_mapHeight = 0;

    /// 返回 GID 所属 tileset 的索引，-1 表示越界或空 tile。
    [[nodiscard]] int findTilesetIndex(std::uint32_t gid) const;

    /// 递归展平 layer tree，累积不透明度和可见性。
    void flattenLayers(const tmx::Map& map, std::vector<FlatEntry>& out);
    void flattenGroup(const tmx::LayerGroup& group, std::vector<FlatEntry>& out,
                      float parentOpacity, bool parentVisible);
};
