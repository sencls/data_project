# 地铁站人群流动仿真系统

基于 C++ 后端 + HTML/Canvas 前端的地铁站人群流动实时仿真系统，数据结构课程大作业项目。

## 功能特性

- **站点拓扑建模** — 使用有向图（邻接表）表示地铁站内节点（入口、售票机、安检口、闸机、通道、楼梯、扶梯、站台、出口等）及其连接关系
- **乘客行为仿真** — 有限状态机驱动乘客完成进站→购票→安检→闸机→站台→出站完整流程，支持正弦波时间分布生成高峰客流
- **实时人群流动** — 离散时间步仿真引擎，考虑拥挤度对移动速度的影响，密度传播与瓶颈检测
- **多种寻路算法** — Dijkstra 最短路径、A* 启发式搜索、多目标 Pareto 优化（时间/拥挤度/区域切换）
- **浏览器可视化** — Canvas 实时渲染站点拓扑、乘客位置、拥挤度光晕、热力图，支持鼠标悬停查看详情
- **交互式控制** — 启动/暂停/重置仿真，可调速度倍率（1x-20x），图层开关

## 项目结构

```
data_project/
├── data_project.sln                    # VS 解决方案
├── data_project/
│   ├── main.cpp                        # 程序入口、HTTP 服务器、仿真主循环
│   ├── include/
│   │   ├── core/                       # 核心数据模型
│   │   │   ├── Types.h                 # 枚举定义（NodeType、PassengerState）
│   │   │   ├── Node.h                  # 站点节点
│   │   │   ├── Edge.h                  # 通道边
│   │   │   ├── Graph.h                 # 有向图（邻接表）
│   │   │   └── Config.h               # 仿真配置参数
│   │   ├── datastructure/              # 自定义数据结构
│   │   │   ├── DynamicArray.h          # 动态数组
│   │   │   ├── HashMap.h              # 哈希表（开放寻址法）
│   │   │   ├── PriorityQueue.h         # 二叉最小堆
│   │   │   └── Queue.h                # 循环缓冲区队列
│   │   ├── simulation/                 # 仿真引擎
│   │   │   ├── Passenger.h            # 乘客实体与状态机
│   │   │   ├── PassengerFactory.h     # 乘客生成器
│   │   │   ├── MovementEngine.h       # 移动引擎
│   │   │   ├── DensityMap.h           # 密度计算
│   │   │   └── Simulator.h            # 仿真调度器
│   │   ├── pathfinding/                # 寻路算法
│   │   │   ├── Pathfinder.h           # Dijkstra / A*
│   │   │   └── MultiObjective.h       # 多目标 Pareto 优化
│   │   ├── analysis/                   # 数据分析
│   │   │   ├── Statistics.h           # 统计指标
│   │   │   └── HeatMap.h              # 热力图
│   │   ├── io/                         # 输入输出
│   │   │   ├── StationLoader.h        # JSON 站点加载
│   │   │   └── JsonSerializer.h       # 状态序列化
│   │   └── server/
│   │       └── HttpServer.h           # HTTP 服务器
│   ├── data/
│   │   └── station_simple.json         # 测试站点（25 节点）
│   ├── frontend/
│   │   ├── index.html
│   │   ├── css/style.css
│   │   └── js/
│   │       ├── app.js                 # 主逻辑
│   │       ├── renderer.js            # Canvas 渲染
│   │       ├── heatmap.js             # 热力图
│   │       └── api.js                 # HTTP 请求
│   └── third_party/
│       ├── httplib.h                  # cpp-httplib
│       └── nlohmann/json.hpp          # JSON for Modern C++
```

## 核心数据结构

| 组件 | 数据结构 | 复杂度 | 用途 |
|------|---------|--------|------|
| 站点图 | `HashMap<int, Node>` + 邻接表 | 查找 O(1) | 稀疏有向图表示 |
| 乘客存储 | `HashMap<int, Passenger>` | 查找 O(1) | 按 ID 管理乘客 |
| 寻路队列 | `PriorityQueue`（二叉最小堆） | push/pop O(log n) | Dijkstra/A* 核心 |
| 节点排队 | `Queue`（循环缓冲区） | 入队/出队 O(1) | 服务节点 FIFO 队列 |
| 乘客行为 | 枚举状态机 | 转换 O(1) | 行为建模 |

## 乘客状态机

```
ENTERING → BUYING_TICKET → SECURITY_CHECK → AT_GATE
    → HEADING_TO_PLATFORM → WAITING_TRAIN → BOARDING → EXITING → DONE
```

每位乘客拥有独立属性：速度倍率、耐心度、熟悉度，影响路径选择与拥堵响应行为。

## 通信架构

C++ 后端内嵌 HTTP 服务器，前端通过轮询获取数据：

| 端点 | 方法 | 功能 |
|------|------|------|
| `/` | GET | 返回前端页面 |
| `/api/station` | GET | 站点图结构 |
| `/api/state` | GET | 仿真快照（乘客位置、密度） |
| `/api/stats` | GET | 统计数据 |
| `/api/start` | GET | 启动仿真 |
| `/api/pause` | GET | 暂停仿真 |
| `/api/reset` | GET | 重置仿真 |
| `/api/speed` | POST | 设置速度倍率 |

线程安全：仿真主线程与 HTTP 服务线程通过 `std::mutex` 保护共享状态。

## 构建与运行

### 环境要求

- Visual Studio 2022（v143 工具集）
- C++17
- Windows（使用 Winsock2）

### 构建步骤

1. 用 Visual Studio 打开 `data_project.sln`
2. 确认项目属性中：
   - C/C++ → 语言 → C++17
   - C/C++ → 命令行 → 附加选项包含 `/utf-8`
   - 链接器 → 输入 → 附加依赖项包含 `ws2_32.lib`
3. 生成解决方案（Ctrl+B）
4. 运行（F5 或 Ctrl+F5）

### 启动

程序启动后控制台输出：

```
============================================
   Subway Crowd Flow Simulation System
============================================
Loading station: data/station_simple.json
Station loaded: 25 nodes, 31 edges
Open http://localhost:8080 in your browser
Press Ctrl+C to stop
```

在浏览器中访问 `http://localhost:8080` 即可看到可视化界面。点击「启动」按钮开始仿真。

### 命令行参数

```
data_project.exe [站点JSON路径]
```

默认加载 `data/station_simple.json`。

## 站点数据格式

站点以 JSON 描述，包含节点和边：

```json
{
  "name": "测试地铁站",
  "nodes": [
    { "id": 1, "type": "entrance", "name": "A入口", "x": 50, "y": 100, "floor": 0,
      "capacity": 50, "area": 40, "speed": 1.5 }
  ],
  "edges": [
    { "from": 1, "to": 3, "length": 8, "width": 4 }
  ]
}
```

节点类型：`entrance`、`ticket`、`security`、`gate`、`corridor`、`stairs`、`escalator`、`platform`、`exit`、`hall`、`transfer`

## 拥挤度模型

```
有效速度 = 节点自由流速 × max(0, 1 - 当前密度 / 容量)
```

密度每 10 个 tick 向相邻节点传播，拥堵时耐心度低的乘客会概率性重新计算路径。
