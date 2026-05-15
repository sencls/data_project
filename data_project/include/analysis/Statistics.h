#pragma once
#include "core/Graph.h"
#include "datastructure/HashMap.h"
#include "datastructure/DynamicArray.h"

struct TimeSeriesPoint {
    int tick;
    double value;
};

class Statistics {
public:
    void recordTick(int tick, Graph& graph, int activePassengers) {
        tickData_.push_back({tick, static_cast<double>(activePassengers)});
        DynamicArray<int> ids = graph.getAllNodeIds();
        for (int i = 0; i < ids.size(); i++) {
            Node* node = graph.getNodePtrMut(ids[i]);
            if (node) {
                nodeHistory_[ids[i]].push_back({tick, node->getDensity()});
            }
        }
    }

    DynamicArray<int> getCongestedNodes(Graph& graph, double threshold = 0.7) {
        DynamicArray<int> result;
        DynamicArray<int> ids = graph.getAllNodeIds();
        for (int i = 0; i < ids.size(); i++) {
            Node* node = graph.getNodePtrMut(ids[i]);
            if (node && node->congestionCoeff >= threshold) {
                result.push_back(ids[i]);
            }
        }
        return result;
    }

    void clear() {
        tickData_.clear();
        nodeHistory_.clear();
    }

private:
    DynamicArray<TimeSeriesPoint> tickData_;
    HashMap<int, DynamicArray<TimeSeriesPoint>> nodeHistory_;
};
