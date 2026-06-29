# 游戏设计文档

## 1. 概述

### 1.1 游戏类型

Vampire Survivors-like（"子弹天堂" / 幸存者割草），俯视角 2D 自动战斗游戏。

### 1.2 核心循环

```
移动躲避 → 武器自动攻击 → 击杀敌人 → 拾取经验宝石 → 升级选技能 → 变强 → 面对更多敌人 → 最终死亡 → 重新开始
```

没有胜利条件。尽可能存活更久。

### 1.3 技术栈

| 项目 | 说明 |
|------|------|
| 语言 | C++17 |
| 图形 | SFML 3.1（静态链接） |
| 音频 | SFML 3.1 Audio 模块 |
| 构建 | CMake >= 3.16 |
| 字体 | DejaVuSans.ttf |
| 音效 | 合成 WAV（Python 生成） |
| 分辨率 | 1920×1080 (16:9) |

### 1.4 操作

| 按键 | 功能 |
|------|------|
| WASD / 方向键 | 玩家移动 |
| ↑↓ / W/S | 升级/暂停菜单选择 |
| Enter / Space | 确认选择 |
| 1 / 2 / 3 | 升级直接选择 |
| Escape | 暂停菜单 |

---

## 2. 架构

### 2.1 目录结构

```
src/
├── main.cpp
├── core/                    # 引擎层
│   ├── Game.hpp/.cpp        # 游戏循环、场景管理、资源加载
│   ├── Scene.hpp/.cpp       # 场景抽象基类
│   ├── Pool.hpp             # 通用对象池（freelist + generation handles）
│   └── ResourceManager.hpp  # 资源缓存模板
├── data/                    # 纯数据定义（header-only）
│   ├── Constants.hpp        # 游戏常量（分辨率、世界大小、敌人属性表等）
│   ├── EntityTypes.hpp      # Enemy / Projectile / XPGem 结构体
│   ├── PlayerState.hpp      # 玩家状态结构体
│   └── Collision.hpp        # 圆-圆碰撞检测
├── systems/                 # 游戏系统
│   ├── WeaponDefs.hpp/.cpp  # 武器定义表 + 等级缩放
│   ├── WeaponSystem.hpp/.cpp# 武器槽管理 + 自动攻击
│   ├── CollisionSystem.hpp/.cpp # 碰撞检测 + 实体清理
│   ├── SpawningSystem.hpp/.cpp  # 敌人生成 + 波次管理
│   ├── UpgradeDefs.hpp/.cpp # 升级选项生成 + 应用
│   ├── WorldRenderer.hpp/.cpp   # 世界空间渲染
│   ├── HUD.hpp/.cpp         # 屏幕空间 UI
│   └── SoundPlayer.hpp/.cpp # 音效池
└── scenes/                  # 场景
    ├── PlayScene.hpp/.cpp   # 主游戏
    ├── TitleScene.hpp/.cpp  # 标题画面
    ├── GameOverScene.hpp/.cpp # 结算画面
    ├── UpgradeUI.hpp/.cpp   # 升级选择界面
    └── PauseMenu.hpp/.cpp   # 暂停菜单
```

### 2.2 设计原则

**数据导向，非对象导向**
- 实体是纯 `struct`，存在 `Pool<T>` 的连续内存中
- 系统函数操作数据，无虚函数、无 Entity 基类
- 优势：cache 友好，热路径零堆分配

**固定时间步**
- 所有逻辑在 60Hz `update(dt)` 中执行
- Glenn Fiedler "Fix Your Timestep" 累加器模式
- 死亡螺旋保护：每帧最多 4 次更新

**枚举分发**
- 武器类型、敌人类型、升级类型都用 `enum + switch`
- 固定集合，编译器可优化为跳转表

**模块化场景**
- PlayScene 只做编排（输入→武器→AI→碰撞→生成→相机→UI）
- 每个子系统职责单一、可独立理解

### 2.3 数据流（一帧）

```
update(dt = 1/60s)
├── 玩家输入（sf::Keyboard 轮询 WASD）
├── 玩家移动 + 世界边界钳制
├── WeaponSystem::update()
│   ├── 各武器冷却递减
│   ├── AoE 武器：对范围内敌人直接造成伤害
│   └── 弹幕武器：索敌→发射→播放射击音效
├── 敌人 AI（所有敌人向玩家直线移动）
├── 弹幕移动（直线或轨道）+ 寿命递减
├── XP 宝石移动（磁铁延迟后加速飞向玩家）
├── CollisionSystem::processCollisions()
│   ├── 弹幕↔敌人 → 伤害 + 穿透 + 击杀时掉落宝石 + 音效
│   ├── 玩家↔敌人 → 伤害 + 无敌帧 + 音效
│   ├── 玩家↔宝石 → 拾取 + 音效
│   └── 清理死实体
├── SpawningSystem::update()
│   ├── Boss 每 60 秒登场
│   ├── 波次生成（类型比例随时间解锁）
│   └── 难度递增（间隔缩短 + 数量增加）
├── 相机跟随玩家
├── 死亡检查（HP≤0 → GameOverScene）
└── 升级检查（XP≥阈值 → 暂停 + 生成选项）
```

---

## 3. 核心系统

### 3.1 对象池 — Pool\<T\>

通用 freelist 池，generation-counter 句柄防悬垂引用。

```cpp
Handle { idx: u32, gen: u32 }  // gen==0 表示空闲
acquire() → Handle              // 优先 freelist，否则扩容
release(Handle)                 // 验证 generation，标记为空
forEach(fn)                     // 遍历所有占用槽位
forEachHandle(fn)               // 遍历，可安全 release
activeCount()                   // 占用数
```

实体不存 `alive` 标志——槽位的 `gen` 字段区分占用/空闲。

### 3.2 玩家

```cpp
PlayerState {
    pos, vel           // 位置 + 输入方向（归一化）
    speed, hp, maxHp, radius
    armor              // 减伤比例 0~0.5
    magnetRange        // 宝石拾取/磁铁范围
    level, xp, xpToNext
    invincibilityTimer // >0 无敌，受击时 =0.5s
}
```

### 3.3 武器系统

5 种武器，通过 `WeaponType` enum 分发。

| 武器 | 行为 | Lv1 冷却 | Lv1 伤害 | 特点 |
|------|------|---------|---------|------|
| MagicWand | 向最近敌人发射追踪弹 | 0.8s | 10 | 快速单体 |
| Knife | 向敌方向扇形发射飞刀 | 1.0s | 8 | 穿透 |
| Axe | 生成环绕玩家旋转的斧头 | 2.0s | 25 | 轨道运动 |
| Fireball | 慢速火球，首次命中消失 | 1.5s | 20 | 高伤 |
| Garlic | 持续 AoE，无弹幕 | 0.5s | 5 | 近身范围伤 |

**等级缩放公式**（1-8 级）：
```
cooldown(N)  = baseCooldown  × 0.95^(N-1)
damage(N)    = baseDamage    × 1.30^(N-1)
pierce(N)    = basePierce    + (N-1)/3
projectileCount(N) = baseCount + (N-1)/2
```

**自动攻击流程**：冷却递减 → 寻找最近敌人 → 发射弹幕（若找到目标）。

Axe 弹幕使用 `orbitAngle/orbitRadius/orbitSpeed` 字段环绕玩家，而非直线运动。

### 3.4 敌人系统

4 种敌人，简单 AI：每帧向玩家位置移动。

| 类型 | HP | 速度 | 伤害/s | 半径 | XP | 出现 |
|------|-----|------|--------|------|-----|------|
| Basic | 20 | 80 | 10 | 14 | 1 | 0:00 |
| Fast | 10 | 160 | 8 | 10 | 2 | 0:30 |
| Tank | 80 | 50 | 20 | 22 | 5 | 1:00 |
| Boss | 300 | 60 | 30 | 32 | 50 | 每60s |

难度递增：生成间隔从 3s 逐渐缩短至 0.5s，每波数量随时间增多。上限 200 只保护帧率。

### 3.5 经验与升级

```
击杀敌人 → 掉落 XP 宝石 → 1.5s 后磁铁吸引 → 玩家接触 → 加 XP
XP ≥ 阈值 → 暂停 → 生成 3 个随机选项 → 玩家选择 → 应用 → 继续
```

**升级选项三类**：
- **新武器**：未拥有且槽位 < 6
- **武器升级**：已拥有且 < Lv.8，显示前后数值对比
- **属性提升**：生命、速度、护甲、磁铁、贪婪（XP 加成）

### 3.6 碰撞检测

朴素 O(N×M) 圆-圆检测，每帧全部检查，然后 `forEachHandle` 清理死实体。

弹幕 vs 敌人 → 敌人 vs 玩家 → 玩家 vs 宝石 → 三波清理。

### 3.7 音效系统

`SoundPlayer` — 24 个 `sf::Sound` 对象池，轮转复用。池满时跳过播放（丢帧不丢音质）。

| 音效 | 触发时机 |
|------|---------|
| shoot | 武器成功发射弹幕 |
| hit | 弹幕命中敌人 |
| kill | 敌人死亡 |
| hurt | 玩家受击 |
| pickup | 拾取 XP 宝石 |
| levelup | 升级 |

### 3.8 渲染管线

```
window.setView(m_camera)       → WorldRenderer: 网格 + 敌人 + 弹幕 + 宝石 + 玩家
window.setView(defaultView)    → HUD: HP 条 + XP 条 + 等级 + 计时器 + 武器列表
if paused                       → UpgradeUI::draw() / PauseMenu::draw()
```

世界 3840×2160，视口 1920×1080，相机跟随玩家并钳制到世界边界。UI 坐标全部用 `VIEW_WIDTH/HEIGHT` 比例计算，换分辨率只需改常量。

---

## 4. 场景流程

```
TitleScene ──Enter──→ PlayScene ──死亡──→ GameOverScene
                              ↑                    │
                              └──── Enter ─────────┘
                              (Esc 暂停菜单可返回标题)
```

| 场景 | 功能 |
|------|------|
| TitleScene | 标题 + 操作说明 + Enter 开始 |
| PlayScene | 主游戏循环，编排所有子系统 |
| GameOverScene | 显示击杀数/等级/存活时间 + Enter 重来 |

延迟场景切换：`Game::changeScene()` 将新场景存入 `m_pendingScene`，在帧末尾安全交换，确保不会在 `update()` 调用栈中销毁当前场景。

---

## 5. 当前数值总览

| 类别 | 参数 | 值 |
|------|------|-----|
| 视口 | 分辨率 | 1920×1080 |
| 世界 | 大小 | 3840×2160 |
| 玩家 | 半径 | 16 |
| 玩家 | 速度 | 220 px/s |
| 玩家 | 初始 HP | 100 |
| 玩家 | 无敌时间 | 0.5s |
| 玩家 | 初始磁铁范围 | 80 |
| 敌人 | 最大数量 | 200 |
| 敌人 | 生成距离 | 1200 |
| 敌人 | 基础波间隔 | 3s |
| 敌人 | 最小波间隔 | 0.5s |
| Boss | 间隔 | 60s |
| XP | 基础升级需求 | 10 |
| XP | 每级增长 | 5 |
| XP | 磁铁延迟 | 1.5s |
| XP | 磁铁速度 | 400 px/s |
| 武器 | 最大槽位 | 6 |
| 武器 | 最大等级 | 8 |
| 音效 | 对象池 | 24 |

---

## 6. 扩展方向

### 短期
- 屏幕震动（受击/Boss 反馈）
- 伤害数字（敌人头顶浮出数值）
- 精灵图替换几何图形
- BGM 背景音乐

### 中期
- 新武器（闪电链、圣水、圣经）
- 被动物品系统（6 槽位 + 属性加成 + 进化条件）
- 武器进化（满级武器 + 对应被动 = 超级形态）
- 更多敌人（远程、自爆、护盾精英）

### 长期
- 多角色可选
- 多关卡/地图
- 局外成长（金币 + 永久升级）
