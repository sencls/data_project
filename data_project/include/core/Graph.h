#pragma once
#include "core/Types.h"
#include "core/Node.h"
#include "core/Edge.h"
#include "datastructure/HashMap.h"
#include "datastructure/DynamicArray.h"

class Graph {
public:
    void addNode(const Node& node) {
        nodes_[node.id] = node;
        if (!adjList_.contains(node.id)) {
            adjList_[node.id] = DynamicArray<Edge>();
        }
        if (!reverseAdj_.contains(node.id)) {
            reverseAdj_[node.id] = DynamicArray<int>();
        }
    }

    void addEdge(const Edge& edge) {
        adjList_[edge.from].push_back(edge);
        reverseAdj_[edge.to].push_back(edge.from);
    }

    Node& getNode(int id) { return nodes_[id]; }
    const Node* getNodePtr(int id) const { return nodes_.get(id); }
    Node* getNodePtrMut(int id) { return nodes_.get(id); }

    DynamicArray<Edge>& getOutEdges(int id) { return adjList_[id]; }

    DynamicArray<int>& getReverseAdj(int id) { return reverseAdj_[id]; }

    bool hasNode(int id) const { return nodes_.contains(id); }

    int nodeCount() { return nodes_.size(); }

    int edgeCount() {
        int total = 0;
        auto keys = nodes_.keys();
        for (int i = 0; i < keys.size(); i++) {
            total += adjList_.contains(keys[i]) ? adjList_[keys[i]].size() : 0;
        }
        return total;
    }

    DynamicArray<int> getAllNodeIds() { return nodes_.keys(); }

    DynamicArray<Node> getAllNodes() { return nodes_.values(); }

    DynamicArray<Edge> getAllEdges() {
        DynamicArray<Edge> result;
        auto keys = nodes_.keys();
        for (int i = 0; i < keys.size(); i++) {
            if (adjList_.contains(keys[i])) {
                auto& edges = adjList_[keys[i]];
                for (int j = 0; j < edges.size(); j++) {
                    result.push_back(edges[j]);
                }
            }
        }
        return result;
    }

    DynamicArray<int> getEntranceIds() {
        DynamicArray<int> result;
        auto keys = nodes_.keys();
        for (int i = 0; i < keys.size(); i++) {
            if (nodes_[keys[i]].type == NodeType::ENTRANCE) {
                result.push_back(keys[i]);
            }
        }
        return result;
    }

    DynamicArray<int> getExitIds() {
        DynamicArray<int> result;
        auto keys = nodes_.keys();
        for (int i = 0; i < keys.size(); i++) {
            if (nodes_[keys[i]].type == NodeType::STATION_EXIT) {
                result.push_back(keys[i]);
            }
        }
        return result;
    }

    DynamicArray<int> getPlatformIds() {
        DynamicArray<int> result;
        auto keys = nodes_.keys();
        for (int i = 0; i < keys.size(); i++) {
            if (nodes_[keys[i]].type == NodeType::PLATFORM) {
                result.push_back(keys[i]);
            }
        }
        return result;
    }

private:
    HashMap<int, Node> nodes_;
    HashMap<int, DynamicArray<Edge>> adjList_;
    HashMap<int, DynamicArray<int>> reverseAdj_;
};
