#pragma once
#include "Types.h"
#include <string>

struct Node {
    int id = -1;
    NodeType type = NodeType::CORRIDOR;
    Position pos;
    std::string name;

    double capacity = 100.0;
    double freeFlowSpeed = 1.5;
    double area = 20.0;
    double congestionCoeff = 0.0;

    int currentOccupancy = 0;

    double getEffectiveSpeed() const {
        double density = (area > 0) ? currentOccupancy / area : 0;
        double maxDensity = (area > 0) ? capacity / area : 5.0;
        double factor = 1.0 - density / maxDensity;
        return freeFlowSpeed * (factor > 0 ? factor : 0);
    }

    double getDensity() const {
        return (area > 0) ? currentOccupancy / area : 0;
    }

    bool isFull() const {
        return currentOccupancy >= static_cast<int>(capacity);
    }
};
