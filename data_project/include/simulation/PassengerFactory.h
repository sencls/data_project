#pragma once
#include "simulation/Passenger.h"
#include "core/Graph.h"
#include "pathfinding/Pathfinder.h"
#include <cmath>
#include <cstdlib>

class PassengerFactory {
public:
    PassengerFactory() : nextId_(1), base_(2), amplitude_(8), peakTick_(500) {}

    void setSchedule(int base, int amplitude, int peakTick) {
        base_ = base;
        amplitude_ = amplitude;
        peakTick_ = peakTick;
    }

    DynamicArray<Passenger> generate(Graph& graph, int tick, int currentPassengerCount = 0) {
        DynamicArray<Passenger> result;
        int count = getGenerationCount(tick);

        // Respect max passenger limit
        const int MAX_PASSENGERS = 1000;
        int remaining = MAX_PASSENGERS - currentPassengerCount;
        if (remaining <= 0) return result;
        if (count > remaining) count = remaining;

        DynamicArray<int> entrances = graph.getEntranceIds();
        DynamicArray<int> platforms = graph.getPlatformIds();
        DynamicArray<int> exits = graph.getExitIds();

        if (entrances.empty() || (platforms.empty() && exits.empty())) return result;

        for (int i = 0; i < count; i++) {
            Passenger p;
            p.id = nextId_++;
            p.originNodeId = entrances[rand() % entrances.size()];
            p.speedMultiplier = 0.8 + (rand() % 40) / 100.0;
            p.patience = 0.3 + (rand() % 70) / 100.0;
            p.familiarity = 0.2 + (rand() % 80) / 100.0;

            if ((rand() % 100) < 70 && !platforms.empty()) {
                p.destNodeId = platforms[rand() % platforms.size()];
            } else if (!exits.empty()) {
                p.destNodeId = exits[rand() % exits.size()];
            } else {
                continue;
            }

            PathResult pathResult = Pathfinder::dijkstra(graph, p.originNodeId, p.destNodeId);
            if (pathResult.found) {
                p.setPath(pathResult.path);
                p.state = PassengerState::ENTERING;
                result.push_back(p);
            }
        }

        return result;
    }

private:
    int nextId_;
    int base_;
    int amplitude_;
    int peakTick_;

    int getGenerationCount(int tick) const {
        double wave = std::sin(static_cast<double>(tick) / peakTick_ * 3.14159);
        int count = base_ + static_cast<int>(amplitude_ * wave);
        return count > 0 ? count : 0;
    }
};
