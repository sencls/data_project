#pragma once
#include "core/Graph.h"
#include "datastructure/HashMap.h"

class DensityMap {
public:
    void update(Graph& graph) {
        auto ids = graph.getAllNodeIds();
        for (int i = 0; i < ids.size(); i++) {
            Node* node = graph.getNodePtrMut(ids[i]);
            if (node) {
                double density = node->getDensity();
                double maxDensity = (node->area > 0) ? node->capacity / node->area : 5.0;
                node->congestionCoeff = (maxDensity > 0) ? density / maxDensity : 0;
                if (node->congestionCoeff > 1.0) node->congestionCoeff = 1.0;
            }
        }
    }

    void propagateCongestion(Graph& graph) {
        auto ids = graph.getAllNodeIds();
        HashMap<int, double> newCong;
        for (int i = 0; i < ids.size(); i++) {
            newCong[ids[i]] = graph.getNode(ids[i]).congestionCoeff;
        }

        for (int i = 0; i < ids.size(); i++) {
            int nodeId = ids[i];
            Node& node = graph.getNode(nodeId);
            double avg = node.congestionCoeff;
            DynamicArray<Edge>& edges = graph.getOutEdges(nodeId);
            int count = 1;
            for (int j = 0; j < edges.size(); j++) {
                Node* neighbor = graph.getNodePtrMut(edges[j].to);
                if (neighbor) {
                    avg += neighbor->congestionCoeff * 0.3;
                    count++;
                }
            }
            newCong[nodeId] = avg / count;
        }

        for (int i = 0; i < ids.size(); i++) {
            Node* node = graph.getNodePtrMut(ids[i]);
            if (node) {
                node->congestionCoeff = newCong[ids[i]];
                if (node->congestionCoeff > 1.0) node->congestionCoeff = 1.0;
            }
        }
    }
};
