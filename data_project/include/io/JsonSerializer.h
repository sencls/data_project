#pragma once
#include "core/Graph.h"

class JsonSerializer {
public:
    static std::string serializeGraph(Graph& graph) {
        std::string json = "{\"nodes\":[";
        DynamicArray<int> ids = graph.getAllNodeIds();
        for (int i = 0; i < ids.size(); i++) {
            Node& node = graph.getNode(ids[i]);
            if (i > 0) json += ",";
            json += "{";
            json += "\"id\":" + std::to_string(node.id) + ",";
            json += "\"type\":\"" + nodeTypeToString(node.type) + "\",";
            json += "\"name\":\"" + node.name + "\",";
            json += "\"x\":" + std::to_string(node.pos.x) + ",";
            json += "\"y\":" + std::to_string(node.pos.y) + ",";
            json += "\"floor\":" + std::to_string(node.pos.floor) + ",";
            json += "\"capacity\":" + std::to_string(static_cast<int>(node.capacity)) + ",";
            json += "\"area\":" + std::to_string(static_cast<int>(node.area)) + ",";
            json += "\"occupancy\":" + std::to_string(node.currentOccupancy) + ",";
            json += "\"density\":" + std::to_string(node.getDensity()) + ",";
            json += "\"congestion\":" + std::to_string(node.congestionCoeff);
            json += "}";
        }
        json += "],\"edges\":[";

        DynamicArray<Edge> edges = graph.getAllEdges();
        for (int i = 0; i < edges.size(); i++) {
            Edge& e = edges[i];
            if (i > 0) json += ",";
            json += "{";
            json += "\"from\":" + std::to_string(e.from) + ",";
            json += "\"to\":" + std::to_string(e.to) + ",";
            json += "\"length\":" + std::to_string(static_cast<int>(e.length)) + ",";
            json += "\"width\":" + std::to_string(static_cast<int>(e.width)) + ",";
            json += "\"load\":" + std::to_string(e.currentLoad);
            json += "}";
        }
        json += "]}";
        return json;
    }
};
