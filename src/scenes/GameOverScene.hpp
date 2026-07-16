#pragma once

#include "core/Scene.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

class Game;

/// 独立的传统结算场景，显示游戏结束统计并允许重新开始。
class GameOverScene : public Scene {
public:
    /// 用结算数据和游戏资源创建结束界面。
    GameOverScene(Game& game, int score, int level, float survivalTime);

    /// 处理重新开始和退出按键。
    void handleEvent(const sf::Event& event) override;
    /// 结束界面没有持续逻辑更新。
    void update(sf::Time dt) override;
    /// 绘制结束标题、统计与操作提示。
    void render(sf::RenderWindow& window) override;

private:
    /// 用于场景切换和关闭窗口的游戏实例。
    Game& m_game;
    /// 默认 UI 字体；资源加载失败时为空。
    const sf::Font* m_font = nullptr;

    /// 游戏结束标题文本。
    sf::Text m_titleText;
    /// 击杀数量统计文本。
    sf::Text m_scoreText;
    /// 达到等级统计文本。
    sf::Text m_levelText;
    /// 存活时间统计文本。
    sf::Text m_timeText;
    /// 重新开始提示文本。
    sf::Text m_restartText;

    /// 本局击杀的敌人数量。
    int m_score;
    /// 本局达到的玩家等级。
    int m_level;
    /// 本局累计存活时长（秒）。
    float m_survivalTime;
};
