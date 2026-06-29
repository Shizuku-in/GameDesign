# 幸存者割草 — 游戏设计文档

## 1. 游戏概述

### 1.1 游戏类型

Vampire Survivors-like（"子弹天堂" / 幸存者割草），俯视角 2D 自动战斗游戏。

### 1.2 核心循环

```
移动躲避 → 武器自动攻击 → 击杀敌人 → 拾取经验宝石 → 升级选技能 → 变强 → 面对更多敌人 → 最终死亡
```

### 1.3 操作方式

| 按键 | 功能 |
|------|------|
| WASD / 方向键 | 玩家移动 |
| 方向键 / 数字键 1-3 | 升级时选择选项 |
| Enter / Space | 升级时确认选择 |
| Escape | 退出游戏 |

### 1.4 胜利条件

**没有胜利条件** — 游戏的核心是"尽可能存活更久"。敌人数量和强度随时间指数增长，死亡是必然结局。游戏体验在于：每次 run 尝试不同的武器组合，挑战更长的存活时间。

---

## 2. 系统架构

### 2.1 架构总览

```
┌──────────────────────────────────────────────────────┐
│                      Game                            │
│  ┌────────────┐  ┌────────────┐  ┌───────────────┐  │
│  │  EventLoop │  │ SceneMgr   │  │ ResourceMgr   │  │
│  │  (60Hz)    │  │ (延迟切换)  │  │ <sf::Font>   │  │
│  └────────────┘  └────────────┘  └───────────────┘  │
│                         │                            │
│         ┌───────────────┼───────────────┐            │
│         ▼               ▼               ▼            │
│    TitleScene      PlayScene      GameOverScene      │
│                    ┌─────┴─────┐                     │
│                    │   Pools    │                     │
│                    │ ┌────────┐ │                     │
│                    │ │ Enemy  │ │                     │
│                    │ │ Proj.  │ │                     │
│                    │ │ XPGem  │ │                     │
│                    │ └────────┘ │                     │
│                    │  Systems   │                     │
│                    │ ┌────────┐ │                     │
│                    │ │ Weapon │ │                     │
│                    │ │ HUD    │ │                     │
│                    │ │Upgrade │ │                     │
│                    │ └────────┘ │                     │
│                    │  Player    │                     │
│                    └───────────┘                     │
└──────────────────────────────────────────────────────┘
```

### 2.2 设计原则

**数据导向，而非对象导向**

- 实体用纯数据 struct（POD），存在连续内存的 `Pool<T>` 中
- 系统函数直接操作 pool 数据，不通过对象方法
- 无 Entity 基类、无虚函数、无继承层级
- 优势：cache 友好，无虚函数 dispatch 开销，代码路径显式可见

**固定时间步，确定性更新**

- 所有逻辑在固定 60Hz `update(dt)` 中执行
- 同一输入 → 同一结果（便于调试和回放）

**枚举分发，而非策略模式**

- 武器类型、敌人类型、升级类型都用 enum + switch
- 固定集合，无需运行时扩展，编译器可优化为跳转表

---

## 3. 核心系统设计

### 3.1 对象池 — `Pool<T>`

```
┌─────────────────────────────────────┐
│            Pool<T>                   │
│  ┌─────────┬─────────┬─────────┐    │
│  │ Slot 0  │ Slot 1  │ Slot 2  │ ... │
│  │ gen=1   │ gen=0   │ gen=3   │    │
│  │ data    │ (free)  │ data    │    │
│  └─────────┴─────────┴─────────┘    │
│  freelist: [1, ...]                  │
│                                      │
│  Handle = { idx: uint32, gen: uint32 }│
│  gen==0  → 槽位空闲                   │
│  Handle.gen == Slot.gen → 有效引用    │
└─────────────────────────────────────┘
```

**关键操作**：
- `acquire()` → 从 freelist 弹出或扩容，返回 Handle
- `release(Handle)` → 验证 generation，标记 gen=0，推回 freelist
- `forEach(fn)` → 线扫描所有 gen!=0 槽位，调用 fn(data)
- Handle 的 generation 防止了悬垂引用：释放后重新分配同一槽位时 gen 已变

**不需要 `alive` 标志**：槽位的 gen 字段已经区分了占用/空闲状态。

### 3.2 实体类型

```cpp
// 敌人 — 向玩家移动，接触造成伤害
struct Enemy {
    sf::Vector2f pos, vel;
    float hp, maxHp, speed, radius;
    float damage;      // 接触伤害/秒
    float xpValue;     // 死亡掉落 XP
    EnemyType type;    // Basic / Fast / Tank / Boss
};

// 弹幕 — 武器发射，命中敌人造成伤害
struct Projectile {
    sf::Vector2f pos, vel;
    float damage, speed, lifetime, radius;
    int pierceCount;   // 剩余穿透数（0=命中即消失）
};

// 经验宝石 — 敌人死亡掉落，玩家拾取
struct XPGem {
    sf::Vector2f pos;
    float value, radius;
    float magnetTimer; // 倒计时结束后被磁铁吸引
};
```

### 3.3 玩家

```cpp
struct PlayerState {
    sf::Vector2f pos;
    float speed, hp, maxHp, radius;
    float armor;              // 减伤比例 0~1
    float magnetRange;        // 宝石拾取范围
    float xpMultiplier;
    int level;
    float xp, xpToNext;
    float invincibilityTimer; // >0 时处于无敌状态
};
```

- 单例（不入池），直接作为 PlayScene 成员
- 武器槽由 `WeaponSystem` 管理，不放在 PlayerState 中

### 3.4 武器系统

#### 武器类型

| 武器 | 行为 | 特点 |
|------|------|------|
| MagicWand | 向最近敌人发射魔法弹 | 快速、低伤、单体 |
| Knife | 向玩家朝向发射飞刀 | 高速、穿透 |
| Axe | 发射环绕玩家的旋转斧头 | 高伤、慢速、轨道运动 |
| Fireball | 慢速火球，首次命中爆炸 | AoE 溅射 |
| Garlic | 无弹幕，持续伤害周围敌人 | 近身 AoE |

#### 等级缩放（1-8 级）

```
cooldown(N)  = baseCooldown  × 0.95^(N-1)   (最短 0.3×)
damage(N)    = baseDamage    × 1.30^(N-1)
pierce(N)    = basePierce    + (N-1) / 3
projectiles(N) = baseCount   + (N-1) / 2
```

#### 自动攻击流程

```
每帧:
  武器冷却 -= dt
  if 冷却 <= 0:
    寻找最近敌人 (距离 < 射程)
    if 找到:
      计算弹幕朝向
      生成弹幕 (从玩家位置偏移)
      重置冷却
```

### 3.5 敌人系统

#### 敌人类型

| 类型 | HP | 速度 | 伤害 | 半径 | XP | 出现时间 |
|------|-----|------|------|------|-----|----------|
| Basic | 20 | 80 | 10/s | 14 | 1 | 0:00 |
| Fast | 10 | 160 | 8/s | 10 | 2 | 2:00 |
| Tank | 80 | 50 | 20/s | 22 | 5 | 4:00 |
| Boss | 300 | 60 | 30/s | 32 | 50 | 每60s |

#### AI 行为

所有敌人使用相同的简单 AI：

```
每帧:
  dir = normalize(player.pos - enemy.pos)
  enemy.pos += dir × enemy.speed × dt
```

无寻路、无避障、无群体行为 — 直接朝向玩家移动。"被卡住"反而成为自然聚集效果。

#### 生成策略

- 敌人在玩家视野外（距离 >= 600px）随机位置生成
- 基础生成间隔 3s，随时间递减至最低 0.5s
- 每波敌人数随存活时间递增
- 难度曲线：早期仅 Basic，中期混入 Fast/Tank，每分钟出现 Boss

### 3.6 经验与升级

#### XP 流程

```
击杀敌人 → 在死亡位置生成 XPGem(value = enemy.xpValue)
         → 宝石静止 1.5s（magnetTimer 倒计时）
         → 倒计时结束后被玩家磁铁吸引
         → 玩家接触宝石 → 拾取 → 增加 XP
         → XP >= xpToNext → 触发升级
```

#### 升级机制

```
触发升级:
  1. 游戏暂停 (m_paused = true)
  2. 随机生成 3 个升级选项
  3. 玩家用方向键/数字键选择
  4. 按 Enter/Space 确认
  5. 应用升级效果
  6. XP -= xpToNext, xpToNext += 成长值, level++
  7. 游戏继续 (m_paused = false)
```

#### 升级选项类型

| 类别 | 条件 | 示例 |
|------|------|------|
| 新武器 | 武器槽 < 6 且武器未拥有 | "获得 Knife" |
| 武器升级 | 已拥有且等级 < 8 | "Wand Lv.3 → Lv.4" |
| 属性提升 | 始终可选 | "最大 HP +20"、"速度 +10%" |

属性提升选项：
- **生命强化**：最大 HP +20，回复 20 HP
- **迅捷**：移动速度 +10%
- **护甲**：伤害减免 +5%（上限 50%）
- **磁铁**：宝石拾取范围 +30
- **贪婪**：XP 获取 +15%

### 3.7 碰撞检测

**朴素圆-圆碰撞**：

```
玩家 ↔ 敌人：    造成 enemy.damage × dt 伤害，触发无敌帧
弹幕 ↔ 敌人：    造成 projectile.damage 伤害，减少穿透数
玩家 ↔ 宝石：    拾取宝石，增加 XP
```

敌人之间不做碰撞（节省计算，且让敌人自然聚集）。

**性能分析**：
- 典型场景：300 敌人 + 30 弹幕 + 50 宝石
- 敌人×弹幕 = 9000 次圆-圆检查/帧
- 60Hz × 9000 = 540K 次/秒 → 现代 CPU 轻松应对
- 暂不引入空间哈希 / 四叉树

### 3.8 HUD

```
┌──────────────────────────────────────────┐
│ HP ████████████░░░░   Lv.12    ⏱ 05:32  │  ← 顶部
│                                          │
│                                          │
│              [游戏画面]                    │
│                                          │
│                                          │
│ XP ████████░░░░░░░░░░░░░░  45/60         │  ← 底部
│                                          │
│           武器槽: Wand Lv.3  Knife Lv.1  │  ← 右侧
└──────────────────────────────────────────┘
```

- 所有 HUD 元素使用 `window.setView(window.getDefaultView())` 在屏幕空间绘制
- HP 条红色，XP 条黄色，文字白色，半透明深色背景
- 升级时覆盖半透明遮罩 + 居中显示选项卡片

### 3.9 相机系统

- 世界大小 1600×1200，视口 800×600
- `sf::View` 始终跟随玩家，夹紧在世界边界
- 游戏实体在世界空间绘制，HUD 在屏幕空间绘制

---

## 4. 场景流转

```
    ┌──────────┐  Enter   ┌──────────┐  死亡   ┌─────────────┐
    │  Title   │ ───────→ │  Play    │ ──────→ │  GameOver   │
    │  Scene   │          │  Scene   │         │   Scene     │
    └──────────┘          └──────────┘         └──────┬──────┘
         ↑                                            │
         └────────────── Enter ───────────────────────┘
```

**TitleScene**：显示游戏标题 + "Press ENTER to start" + 操作说明
**PlayScene**：主游戏循环，所有 gameplay 逻辑
**GameOverScene**：显示存活时间 + 击杀数，按 Enter 重新开始

### 延迟场景切换

当前代码的 `Game::changeScene()` 是立即销毁旧场景。如果从 `update()` 中调用会导致 use-after-free。修复方式：

```cpp
// Game.hpp 新增
std::unique_ptr<Scene> m_pendingScene;

// Game::changeScene — 存入 pending
void Game::changeScene(std::unique_ptr<Scene> scene) {
    m_pendingScene = std::move(scene);
}

// Game::run() 末尾 — 安全时机执行切换
if (m_pendingScene) {
    m_scene = std::move(m_pendingScene);
    m_pendingScene = nullptr;
}
```

---

## 5. 数据流

### 5.1 一帧内的数据流

```
update(dt = 1/60s)
│
├─ 1. 玩家输入
│   sf::Keyboard::isKeyPressed() → m_player.vel
│   m_player.pos += vel × speed × dt
│   夹紧到世界边界
│
├─ 2. 武器系统
│   for each weapon slot:
│     冷却 -= dt
│     if 冷却完毕:
│       寻找最近敌人 (遍历 Enemies pool)
│       计算朝向 → 生成弹幕 (Projectile pool)
│
├─ 3. 敌人 AI
│   for each Enemy:
│     dir = normalize(player.pos - enemy.pos)
│     enemy.pos += dir × speed × dt
│
├─ 4. 弹幕移动
│   for each Projectile:
│     pos += vel × dt
│     lifetime -= dt
│     if lifetime <= 0: release
│
├─ 5. XP 宝石移动
│   for each XPGem:
│     magnetTimer -= dt
│     if magnetTimer <= 0:
│       向玩家加速移动 (磁铁速度)
│
├─ 6. 碰撞检测
│   for Enemy × Projectile: → 伤害、穿透、击杀
│   for Enemy × Player:     → 伤害、无敌帧
│   for XPGem × Player:     → 拾取、加 XP
│
├─ 7. 波次生成
│   spawnTimer -= dt
│   if <= 0: 在视野外生成一批敌人, 重置计时器
│
├─ 8. 升级检查
│   if player.xp >= player.xpToNext:
│     m_paused = true
│     生成 3 个升级选项
│
└─ 9. 死亡检查
    if player.hp <= 0:
      → Game::changeScene(GameOverScene)
```

### 5.2 渲染流

```
render(window)
│
├─ window.clear(darkGray)
├─ window.setView(m_camera)       // 世界空间
│   ├─ Draw all Enemies    (红圆, 半径=Enemy.radius)
│   ├─ Draw all Projectiles(黄圆, 半径=Projectile.radius)
│   ├─ Draw all XPGems     (绿圆, 半径=XPGem.radius)
│   └─ Draw Player         (白圆, 无敌时闪烁)
│
├─ window.setView(window.getDefaultView())  // 屏幕空间
│   ├─ Draw HP Bar, XP Bar, Level, Timer
│   └─ if paused: Draw overlay + upgrade cards
│
└─ window.display()  // Game::render() 负责
```

---

## 6. 文件结构

```
src/
├── main.cpp                 # 入口
├── Game.hpp / Game.cpp      # 游戏循环、场景管理、资源管理
├── Scene.hpp / Scene.cpp    # 场景抽象基类
├── ResourceManager.hpp      # 资源缓存模板 (已有)
│
├── Pool.hpp                 # [新] 通用对象池
├── Constants.hpp            # [新] 游戏常量
├── Collision.hpp            # [新] 碰撞检测函数
│
├── EntityTypes.hpp          # [新] Enemy/Projectile/XPGem 数据定义
├── PlayerState.hpp          # [新] 玩家状态
│
├── WeaponDefs.hpp/cpp       # [新] 武器定义 + 等级缩放
├── WeaponSystem.hpp/cpp     # [新] 武器槽管理 + 自动攻击
│
├── UpgradeDefs.hpp/cpp      # [新] 升级选项生成 + 应用
│
├── HUD.hpp/cpp              # [新] HP条/XP条/等级/计时器
│
├── PlayScene.hpp/cpp        # [新] 主游戏场景
├── TitleScene.hpp/cpp       # [新] 标题画面
└── GameOverScene.hpp/cpp    # [新] 结束画面

assets/
└── fonts/
    └── DejaVuSans.ttf       # UI 字体 (需从 build/assets/fonts/ 拷贝)

docs/
├── wsl-setup-guide.md       # 已有
└── design-doc.md            # 本文档
```

---

## 7. 开发周期

### 总览

| 阶段 | 内容 | 预计工作量 | 可玩性 |
|------|------|-----------|--------|
| M0 | 基础设施 | 小 | — |
| M1 | 数据层 | 小 | — |
| M2 | 核心系统 | 中 | — |
| M3 | 主游戏场景 | 大 | ✅ 首次可玩 |
| M4 | 场景完善 | 小 | ✅ 完整循环 |
| M5 | 打磨 | 中 | ✅ 成品 |

### M0 — 基础设施

**目标**：搭建开发所需的底层设施

- [ ] 修复 `Game::changeScene()` — 延迟场景切换
- [ ] 实现 `Pool<T>` 通用对象池模板
- [ ] 配置 CMakeLists.txt：POST_BUILD 拷贝 assets
- [ ] 拷贝字体文件到 `assets/fonts/DejaVuSans.ttf`
- [ ] 在 `Game` 中添加 `ResourceManager<sf::Font>`

**产出**：编译通过，可运行（但无游戏内容）

### M1 — 数据层

**目标**：定义所有数据结构，无行为逻辑

- [ ] `Constants.hpp` — 集中管理所有可调常量
- [ ] `EntityTypes.hpp` — Enemy / Projectile / XPGem struct
- [ ] `PlayerState.hpp` — 玩家状态 struct
- [ ] `WeaponDefs.hpp/cpp` — 武器定义表 + 缩放公式
- [ ] `Collision.hpp` — 圆-圆碰撞 inline 函数

**产出**：所有数据结构可用，通过编译

### M2 — 核心系统

**目标**：实现独立可测的游戏子系统

- [ ] `WeaponSystem.hpp/cpp` — 寻敌 + 自动攻击 + 弹幕生成
- [ ] `UpgradeDefs.hpp/cpp` — 随机升级生成 + 应用
- [ ] `HUD.hpp/cpp` — HP/XP 条 + 等级 + 计时器渲染

**产出**：各系统可单元测试（手动构造数据验证）

### M3 — 主游戏场景 ⭐

**目标**：`PlayScene` 串联所有系统，实现核心游戏循环

- [ ] 玩家 WASD 移动 + 世界边界夹紧
- [ ] 武器自动攻击（初始 MagicWand Lv.1）
- [ ] 敌人 AI（向玩家移动）
- [ ] 弹幕移动 + 生命周期
- [ ] 碰撞检测（玩家↔敌人、弹幕↔敌人、玩家↔宝石）
- [ ] 敌人波次生成（难度递增）
- [ ] XP 掉落 + 磁铁拾取
- [ ] 升级暂停 + 选项选择
- [ ] 死亡 → GameOver 切换
- [ ] 相机跟随玩家

**产出**：🎮 **首次可玩** — 核心游戏循环完整

### M4 — 场景完善

**目标**：完整的 Title → Play → GameOver 循环

- [ ] `TitleScene` — 标题 + 操作说明 + 按 Enter 开始
- [ ] `GameOverScene` — 显示存活时间/等级 + 按 Enter 重来
- [ ] 删除 `GameScene`（旧 Demo）
- [ ] 更新 `Game` 构造函数，默认启动 TitleScene

**产出**：完整可玩流程，可反复重开

### M5 — 打磨

**目标**：提升视觉反馈和游戏体验

- [ ] 受击闪烁效果（无敌帧时玩家闪烁）
- [ ] 敌人受击反馈（短暂颜色变化/缩放）
- [ ] 武器弹幕多样化（不同颜色/形状区分武器）
- [ ] 屏幕震动（Boss 出场 / 玩家受击）
- [ ] 升级选项 UI 美化（卡片式布局）
- [ ] 数值平衡调整
- [ ] 全量 clang-format 格式化

**产出**：可直接演示的成品

---

## 8. 技术备注

### SFML 3.1 API 要点

- 事件轮询：`while (auto event = window.pollEvent())` 返回 `std::optional`
- 事件判断：`event->is<sf::Event::Closed>()` / `event->getIf<sf::Event::KeyPressed>()`
- 键盘轮询：`sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)`
- 颜色：`sf::Color::Red` / `sf::Color(255,0,0)` （静态成员是 3.1 新增）
- View：`sf::View(sf::FloatRect({0,0}, {800,600}))`

### C++ 规范

- C++17，无扩展
- clang-format：WebKit 风格，4 空格缩进，100 字符行宽
- 警告全开：`-Wall -Wextra -Wpedantic`
- 提交前运行：`cmake --build build --target format`

### 性能约定

- 实体数据连续存储（`std::vector`），避免堆碎片
- 热路径（update 循环中）零堆分配
- `Pool<T>` 的 `forEach` 做线性扫描（cache 友好）
- 无 RTTI、无异常（SFML 可能抛异常，但不主动使用）

### 第三方依赖

仅 SFML 3.1（静态链接：Graphics → Window → System + 平台库）。无需其他库。
