#pragma once
#include "core/Graph.h"
#include <vector>
#include <cmath>

class HeatMap {
public:
    HeatMap() : gridWidth_(50), gridHeight_(50) {}

    void compute(Graph& graph) {
        grid_.assign(gridHeight_, std::vector<double>(gridWidth_, 0.0));

        DynamicArray<int> ids = graph.getAllNodeIds();
        for (int i = 0; i < ids.size(); i++) {
            Node* node = graph.getNodePtrMut(ids[i]);
            if (!node) continue;

            int gx = static_cast<int>(node->pos.x / 20.0);
            int gy = static_cast<int>(node->pos.y / 20.0);
            if (gx < 0) gx = 0;
            if (gy < 0) gy = 0;
            if (gx >= gridWidth_) gx = gridWidth_ - 1;
            if (gy >= gridHeight_) gy = gridHeight_ - 1;

            double density = node->getDensity();
            spread(gx, gy, density, 2);
        }
    }

    int getWidth() const { return gridWidth_; }
    int getHeight() const { return gridHeight_; }
    const std::vector<std::vector<double>>& getGrid() const { return grid_; }

private:
    int gridWidth_;
    int gridHeight_;
    std::vector<std::vector<double>> grid_;

    void spread(int cx, int cy, double value, int radius) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int nx = cx + dx;
                int ny = cy + dy;
                if (nx < 0 || nx >= gridWidth_ || ny < 0 || ny >= gridHeight_) continue;
                double dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));
                if (dist <= radius) {
                    double factor = 1.0 - dist / (radius + 1);
                    grid_[ny][nx] += value * factor;
                }
            }
        }
    }
};
