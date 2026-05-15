#pragma once

struct Edge {
    int from = -1;
    int to = -1;
    double length = 1.0;    // 长度 (m)
    double width = 2.0;     // 宽度 (m)

    // 运行时状态
    int currentLoad = 0;    // 当前通道内人数

    double getFlowRate() const {
        return width * 1.2; // 单位宽度通行率 × 宽度
    }
};
