#pragma once

struct Config {
    int tickIntervalMs = 1000;       // 仿真时间步间隔 (ms)
    int serverPort = 8080;           // HTTP 服务器端口
    int snapshotInterval = 1;        // 每 N 步生成一次快照
    int defaultSpeedMultiplier = 1;  // 默认速度倍率

    double trainInterval = 120.0;    // 列车间隔 (秒/tick)
    double boardingTime = 30.0;      // 上车时间 (秒/tick)

    // 乘客生成参数
    int basePassengersPerTick = 2;
    int peakExtraPassengers = 8;
    int peakTick = 500;
};
