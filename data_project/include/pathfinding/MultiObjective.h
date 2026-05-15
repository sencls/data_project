#pragma once
#include "pathfinding/Pathfinder.h"
#include "core/Graph.h"
#include <string>

struct MultiObjSolution {
    DynamicArray<int> path;
    double time = 0;
    double distance = 0;
    double congestionScore = 0;
    int areaSwitches = 0;
    std::string description;
};

class MultiObjective {
public:
    static DynamicArray<MultiObjSolution> findPaths(Graph& graph, int from, int to) {
        DynamicArray<MultiObjSolution> solutions;

        // 方案1: 最短时间
        PathResult pathTime = weightedDijkstra(graph, from, to, 1.0, 0.0, 0.0, 0.0);
        if (pathTime.found) {
            MultiObjSolution sol;
            sol.path = pathTime.path;
            sol.time = computeTime(graph, pathTime.path);
            sol.distance = computeDistance(graph, pathTime.path);
            sol.congestionScore = computeCongestion(graph, pathTime.path);
            sol.areaSwitches = computeAreaSwitches(graph, pathTime.path);
            sol.description = "最短时间";
            solutions.push_back(sol);
        }

        // 方案2: 最低拥挤
        PathResult pathCong = weightedDijkstra(graph, from, to, 0.3, 0.0, 0.7, 0.0);
        if (pathCong.found) {
            MultiObjSolution sol;
            sol.path = pathCong.path;
            sol.time = computeTime(graph, pathCong.path);
            sol.distance = computeDistance(graph, pathCong.path);
            sol.congestionScore = computeCongestion(graph, pathCong.path);
            sol.areaSwitches = computeAreaSwitches(graph, pathCong.path);
            sol.description = "最低拥挤";
            bool dup = false;
            for (int i = 0; i < solutions.size(); i++) {
                if (pathEqual(sol.path, solutions[i].path)) { dup = true; break; }
            }
            if (!dup) solutions.push_back(sol);
        }

        // 方案3: 最少区域切换
        PathResult pathSwitch = weightedDijkstra(graph, from, to, 0.3, 0.0, 0.0, 0.7);
        if (pathSwitch.found) {
            MultiObjSolution sol;
            sol.path = pathSwitch.path;
            sol.time = computeTime(graph, pathSwitch.path);
            sol.distance = computeDistance(graph, pathSwitch.path);
            sol.congestionScore = computeCongestion(graph, pathSwitch.path);
            sol.areaSwitches = computeAreaSwitches(graph, pathSwitch.path);
            sol.description = "最少区域切换";
            bool dup = false;
            for (int i = 0; i < solutions.size(); i++) {
                if (pathEqual(sol.path, solutions[i].path)) { dup = true; break; }
            }
            if (!dup) solutions.push_back(sol);
        }

        return solutions;
    }

private:
    static PathResult weightedDijkstra(Graph& graph, int from, int to,
                                        double wTime, double wDist, double wCong,
                                        double wSwitch) {
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
            int u = top.second;
            if (visited[u]) continue;
            visited[u] = true;
            if (u == to) break;

            if (!graph.hasNode(u)) continue;
            Node* uNode = graph.getNodePtrMut(u);
            DynamicArray<Edge>& edges = graph.getOutEdges(u);

            for (int i = 0; i < edges.size(); i++) {
                Edge& e = edges[i];
                int v = e.to;
                if (visited[v]) continue;

                Node* vNode = graph.getNodePtrMut(v);
                double congFactor = 1.0;
                if (vNode && vNode->getEffectiveSpeed() > 0) {
                    congFactor = vNode->freeFlowSpeed / vNode->getEffectiveSpeed();
                }

                double timeCost = e.length * congFactor;
                double congCost = vNode ? vNode->congestionCoeff * e.length : 0;
                double switchCost = 0;
                if (uNode && vNode && uNode->type != vNode->type) {
                    switchCost = 5.0;
                }

                double totalW = wTime * timeCost + wDist * e.length +
                                wCong * congCost + wSwitch * switchCost;
                double newDist = dist[u] + totalW;
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                    pq.push(std::make_pair(newDist, v));
                }
            }
        }

        if (!dist.contains(to) || dist[to] >= 1e18) { result.found = false; return result; }

        result.cost = dist[to];
        result.found = true;
        DynamicArray<int> reversePath;
        int cur = to;
        while (cur != -1) { reversePath.push_back(cur); cur = prev[cur]; }
        for (int i = reversePath.size() - 1; i >= 0; i--) result.path.push_back(reversePath[i]);
        return result;
    }

    static double computeTime(Graph& g, const DynamicArray<int>& path) {
        double t = 0;
        for (int i = 0; i < path.size() - 1; i++) {
            Node* node = g.getNodePtrMut(path[i + 1]);
            double speed = node ? node->getEffectiveSpeed() : 1.0;
            DynamicArray<Edge>& edges = g.getOutEdges(path[i]);
            for (int j = 0; j < edges.size(); j++) {
                if (edges[j].to == path[i + 1]) {
                    t += (speed > 0) ? edges[j].length / speed : edges[j].length;
                    break;
                }
            }
        }
        return t;
    }

    static double computeDistance(Graph& g, const DynamicArray<int>& path) {
        double d = 0;
        for (int i = 0; i < path.size() - 1; i++) {
            DynamicArray<Edge>& edges = g.getOutEdges(path[i]);
            for (int j = 0; j < edges.size(); j++) {
                if (edges[j].to == path[i + 1]) { d += edges[j].length; break; }
            }
        }
        return d;
    }

    static double computeCongestion(Graph& g, const DynamicArray<int>& path) {
        double score = 0;
        for (int i = 0; i < path.size(); i++) {
            Node* node = g.getNodePtrMut(path[i]);
            if (node) score += node->congestionCoeff;
        }
        return score;
    }

    static int computeAreaSwitches(Graph& g, const DynamicArray<int>& path) {
        int switches = 0;
        for (int i = 1; i < path.size(); i++) {
            Node* a = g.getNodePtrMut(path[i - 1]);
            Node* b = g.getNodePtrMut(path[i]);
            if (a && b && a->type != b->type) switches++;
        }
        return switches;
    }

    static bool pathEqual(const DynamicArray<int>& a, const DynamicArray<int>& b) {
        if (a.size() != b.size()) return false;
        for (int i = 0; i < a.size(); i++) {
            if (a[i] != b[i]) return false;
        }
        return true;
    }
};
