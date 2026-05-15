#pragma once
#include "core/Graph.h"
#include "datastructure/DynamicArray.h"
#include "datastructure/PriorityQueue.h"
#include "datastructure/HashMap.h"
#include <cmath>
#include <utility>

struct PathResult {
    DynamicArray<int> path;
    double cost = 0;
    bool found = false;
};

class Pathfinder {
public:
    static PathResult dijkstra(Graph& graph, int from, int to) {
        PathResult result;
        DynamicArray<int> ids = graph.getAllNodeIds();

        HashMap<int, double> dist;
        HashMap<int, int> prev;
        HashMap<int, bool> visited;

        for (int i = 0; i < ids.size(); i++) {
            dist[ids[i]] = 1e18;
            prev[ids[i]] = -1;
            visited[ids[i]] = false;
        }
        dist[from] = 0;

        auto cmp = [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
            return a.first < b.first;
        };
        PriorityQueue<std::pair<double, int>> pq(cmp);
        pq.push(std::make_pair(0.0, from));

        while (!pq.empty()) {
            std::pair<double, int> top = pq.pop();
            double d = top.first;
            int u = top.second;
            if (visited[u]) continue;
            visited[u] = true;
            if (u == to) break;

            if (!graph.hasNode(u)) continue;
            DynamicArray<Edge>& edges = graph.getOutEdges(u);
            for (int i = 0; i < edges.size(); i++) {
                Edge& e = edges[i];
                int v = e.to;
                if (visited[v]) continue;

                Node* nodeTo = graph.getNodePtrMut(v);
                double congFactor = 1.0;
                if (nodeTo && nodeTo->getEffectiveSpeed() > 0) {
                    congFactor = nodeTo->freeFlowSpeed / nodeTo->getEffectiveSpeed();
                }
                double w = e.length * congFactor;
                double newDist = d + w;
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                    pq.push(std::make_pair(newDist, v));
                }
            }
        }

        if (!dist.contains(to) || dist[to] >= 1e18) {
            result.found = false;
            return result;
        }

        result.cost = dist[to];
        result.found = true;

        DynamicArray<int> reversePath;
        int cur = to;
        while (cur != -1) {
            reversePath.push_back(cur);
            cur = prev[cur];
        }

        for (int i = reversePath.size() - 1; i >= 0; i--) {
            result.path.push_back(reversePath[i]);
        }

        return result;
    }

    static PathResult astar(Graph& graph, int from, int to) {
        PathResult result;
        Node* toNode = graph.getNodePtrMut(to);
        if (!toNode) { result.found = false; return result; }

        DynamicArray<int> ids = graph.getAllNodeIds();
        HashMap<int, double> gScore;
        HashMap<int, double> fScore;
        HashMap<int, int> prev;
        HashMap<int, bool> closed;

        for (int i = 0; i < ids.size(); i++) {
            gScore[ids[i]] = 1e18;
            fScore[ids[i]] = 1e18;
            prev[ids[i]] = -1;
            closed[ids[i]] = false;
        }

        Node* fromNode = graph.getNodePtrMut(from);
        double h0 = heuristic(fromNode, toNode);
        gScore[from] = 0;
        fScore[from] = h0;

        auto cmp = [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
            return a.first < b.first;
        };
        PriorityQueue<std::pair<double, int>> open(cmp);
        open.push(std::make_pair(fScore[from], from));

        while (!open.empty()) {
            std::pair<double, int> top = open.pop();
            double f = top.first;
            int u = top.second;
            if (closed[u]) continue;
            closed[u] = true;
            if (u == to) break;

            if (!graph.hasNode(u)) continue;
            DynamicArray<Edge>& edges = graph.getOutEdges(u);
            for (int i = 0; i < edges.size(); i++) {
                Edge& e = edges[i];
                int v = e.to;
                if (closed[v]) continue;

                Node* nodeTo = graph.getNodePtrMut(v);
                double congFactor = 1.0;
                if (nodeTo && nodeTo->getEffectiveSpeed() > 0) {
                    congFactor = nodeTo->freeFlowSpeed / nodeTo->getEffectiveSpeed();
                }
                double w = e.length * congFactor;
                double tentG = gScore[u] + w;
                if (tentG < gScore[v]) {
                    prev[v] = u;
                    gScore[v] = tentG;
                    Node* vNode = graph.getNodePtrMut(v);
                    fScore[v] = tentG + heuristic(vNode, toNode);
                    open.push(std::make_pair(fScore[v], v));
                }
            }
        }

        if (!gScore.contains(to) || gScore[to] >= 1e18) {
            result.found = false;
            return result;
        }

        result.cost = gScore[to];
        result.found = true;

        DynamicArray<int> reversePath;
        int cur = to;
        while (cur != -1) {
            reversePath.push_back(cur);
            cur = prev[cur];
        }
        for (int i = reversePath.size() - 1; i >= 0; i--) {
            result.path.push_back(reversePath[i]);
        }
        return result;
    }

    static double heuristic(const Node* a, const Node* b) {
        if (!a || !b) return 0;
        double dx = a->pos.x - b->pos.x;
        double dy = a->pos.y - b->pos.y;
        double df = (a->pos.floor - b->pos.floor) * 10.0;
        return std::sqrt(dx * dx + dy * dy + df * df);
    }
};
