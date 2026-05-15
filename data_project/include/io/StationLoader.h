#pragma once
#include "core/Graph.h"
#include <fstream>
#include <iostream>

class StationLoader {
public:
    static bool loadFromFile(const std::string& path, Graph& graph) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Cannot open station file: " << path << std::endl;
            return false;
        }

        // 手动解析 JSON (避免在头文件中引入 nlohmann/json)
        // 简化解析：逐行读取并提取字段
        std::string line;
        std::string content;
        while (std::getline(file, line)) {
            content += line;
        }
        file.close();

        // 使用 nlohmann/json 解析
        // 将解析逻辑放在这里，通过前置声明避免头文件依赖
        return parseStationJson(content, graph);
    }

private:
    static bool parseStationJson(const std::string& content, Graph& graph);
};
