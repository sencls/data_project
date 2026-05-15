#pragma once
#include "core/Types.h"
#include "core/Node.h"
#include "datastructure/DynamicArray.h"

class Passenger {
public:
    int id = -1;
    int originNodeId = -1;
    int destNodeId = -1;
    double speedMultiplier = 1.0;
    double patience = 0.5;
    double familiarity = 0.5;

    PassengerState state = PassengerState::ENTERING;
    int currentNodeId = -1;
    DynamicArray<int> plannedPath;
    int pathIndex = 0;

    double edgeProgress = 0.0;
    double totalTravelTime = 0.0;
    double waitTime = 0.0;

    void setPath(const DynamicArray<int>& path) {
        plannedPath = path;
        pathIndex = 0;
        if (path.size() > 0) {
            currentNodeId = path[0];
        }
    }

    bool advanceOnEdge(double speed, double edgeLength) {
        if (edgeLength <= 0) return true;
        double step = speed / edgeLength;
        edgeProgress += step * speedMultiplier;
        return edgeProgress >= 1.0;
    }

    void moveToNextNode() {
        pathIndex++;
        if (pathIndex < plannedPath.size()) {
            currentNodeId = plannedPath[pathIndex];
            edgeProgress = 0.0;
        }
    }

    bool hasNextNode() const {
        return pathIndex + 1 < plannedPath.size();
    }

    int getNextNodeId() const {
        if (pathIndex + 1 < plannedPath.size()) return plannedPath[pathIndex + 1];
        return -1;
    }

    Position getInterpolatedPosition(const Node& fromNode, const Node& toNode) const {
        Position p;
        p.x = fromNode.pos.x + (toNode.pos.x - fromNode.pos.x) * edgeProgress;
        p.y = fromNode.pos.y + (toNode.pos.y - fromNode.pos.y) * edgeProgress;
        p.floor = fromNode.pos.floor;
        return p;
    }
};
