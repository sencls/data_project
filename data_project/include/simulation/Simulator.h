#pragma once
#include "simulation/Passenger.h"
#include "simulation/PassengerFactory.h"
#include "simulation/MovementEngine.h"
#include "simulation/DensityMap.h"
#include "core/Graph.h"
#include "core/Config.h"
#include "datastructure/HashMap.h"
#include "pathfinding/MultiObjective.h"

#include <mutex>
#include <string>
#include <cmath>
#include <cstdio>

struct SimStats {
    int totalGenerated = 0;
    int totalCompleted = 0;
    int currentActive = 0;
    double avgTravelTime = 0;
    int bottleneckNodeId = -1;
    double maxCongestion = 0;
};

class Simulator {
public:
    Simulator() : running_(false), currentTick_(0), speedMultiplier_(1) {}

    bool loadStation(const std::string& path);
    void start() { running_ = true; }
    void pause() { running_ = false; }

    void reset() {
        running_ = false;
        currentTick_ = 0;
        passengers_.clear();
        stats_ = SimStats();
        DynamicArray<int> ids = graph_.getAllNodeIds();
        for (int i = 0; i < ids.size(); i++) {
            Node* node = graph_.getNodePtrMut(ids[i]);
            if (node) {
                node->currentOccupancy = 0;
                node->congestionCoeff = 0;
            }
        }
    }

    void setSpeed(int mult) { speedMultiplier_ = (mult > 0) ? mult : 1; }

    void step() {
        if (!running_) return;
        currentTick_++;

        // 1. 生成新乘客
        DynamicArray<Passenger> newPassengers = factory_.generate(graph_, currentTick_, passengers_.size());
        for (int i = 0; i < newPassengers.size(); i++) {
            Passenger& p = newPassengers[i];
            passengers_[p.id] = p;
            stats_.totalGenerated++;
            Node* node = graph_.getNodePtrMut(p.currentNodeId);
            if (node) node->currentOccupancy++;
        }

        // 2. 移动乘客
        movement_.advanceAll(passengers_, graph_, currentTick_);

        // 3. 更新密度
        density_.update(graph_);
        if (currentTick_ % 10 == 0) {
            density_.propagateCongestion(graph_);
        }

        // 4. 统计
        stats_.currentActive = passengers_.size();
        updateBottleneck();
    }

    int getCurrentTick() const { return currentTick_; }
    bool isRunning() const { return running_; }
    Graph& getGraph() { return graph_; }
    HashMap<int, Passenger>& getPassengers() { return passengers_; }
    const SimStats& getStats() const { return stats_; }
    int getSpeedMultiplier() const { return speedMultiplier_; }

    std::string serializeState() {
        std::string j = "{";
        j += "\"tick\":" + std::to_string(currentTick_) + ",";
        j += "\"running\":" + std::string(running_ ? "true" : "false") + ",";
        j += "\"speed\":" + std::to_string(speedMultiplier_) + ",";

        int hours = 6 + currentTick_ / 3600;
        int mins = (currentTick_ % 3600) / 60;
        int secs = currentTick_ % 60;
        char timeBuf[16];
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", hours % 24, mins, secs);
        j += "\"time\":\"" + std::string(timeBuf) + "\",";

        // Passengers
        j += "\"passengers\":[";
        DynamicArray<int> pids = passengers_.keys();
        for (int i = 0; i < pids.size(); i++) {
            Passenger& p = passengers_[pids[i]];
            Node* curNode = graph_.getNodePtrMut(p.currentNodeId);
            Node* nextNode = graph_.getNodePtrMut(p.getNextNodeId());

            if (i > 0) j += ",";
            j += "{\"id\":" + std::to_string(p.id) + ",";
            j += "\"state\":\"" + passengerStateToString(p.state) + "\",";
            j += "\"speed\":" + std::to_string(p.speedMultiplier).substr(0, 4) + ",";
            j += "\"patience\":" + std::to_string(p.patience).substr(0, 4) + ",";
            j += "\"familiarity\":" + std::to_string(p.familiarity).substr(0, 4) + ",";
            j += "\"travelTime\":" + std::to_string(static_cast<int>(p.totalTravelTime)) + ",";
            j += "\"waitTime\":" + std::to_string(static_cast<int>(p.waitTime)) + ",";
            j += "\"origin\":" + std::to_string(p.originNodeId) + ",";
            j += "\"dest\":" + std::to_string(p.destNodeId) + ",";

            double px = 0, py = 0;
            if (nextNode && curNode) {
                px = curNode->pos.x + (nextNode->pos.x - curNode->pos.x) * p.edgeProgress;
                py = curNode->pos.y + (nextNode->pos.y - curNode->pos.y) * p.edgeProgress;
            } else if (curNode) {
                px = curNode->pos.x;
                py = curNode->pos.y;
            }
            j += "\"x\":" + std::to_string(px) + ",";
            j += "\"y\":" + std::to_string(py) + "}";
        }
        j += "],";

        // Nodes
        j += "\"nodes\":[";
        DynamicArray<int> nids = graph_.getAllNodeIds();
        for (int i = 0; i < nids.size(); i++) {
            Node& node = graph_.getNode(nids[i]);
            if (i > 0) j += ",";
            j += "{\"id\":" + std::to_string(node.id) + ",";
            j += "\"density\":" + std::to_string(node.getDensity()) + ",";
            j += "\"occupancy\":" + std::to_string(node.currentOccupancy) + ",";
            j += "\"congestion\":" + std::to_string(node.congestionCoeff) + "}";
        }
        j += "],";

        // Stats
        j += "\"stats\":{";
        j += "\"totalGenerated\":" + std::to_string(stats_.totalGenerated) + ",";
        j += "\"totalCompleted\":" + std::to_string(stats_.totalGenerated - stats_.currentActive) + ",";
        j += "\"currentActive\":" + std::to_string(stats_.currentActive) + ",";
        j += "\"bottleneckNodeId\":" + std::to_string(stats_.bottleneckNodeId) + ",";
        j += "\"maxCongestion\":" + std::to_string(stats_.maxCongestion) + "}";

        j += "}";
        return j;
    }

private:
    Graph graph_;
    HashMap<int, Passenger> passengers_;
    PassengerFactory factory_;
    MovementEngine movement_;
    DensityMap density_;
    SimStats stats_;

    bool running_;
    int currentTick_;
    int speedMultiplier_;

    void updateBottleneck() {
        double maxCong = 0;
        int maxNodeId = -1;
        DynamicArray<int> ids = graph_.getAllNodeIds();
        for (int i = 0; i < ids.size(); i++) {
            Node* node = graph_.getNodePtrMut(ids[i]);
            if (node && node->congestionCoeff > maxCong) {
                maxCong = node->congestionCoeff;
                maxNodeId = ids[i];
            }
        }
        stats_.maxCongestion = maxCong;
        stats_.bottleneckNodeId = maxNodeId;
    }
};
