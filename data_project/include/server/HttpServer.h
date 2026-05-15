#pragma once
#include "../simulation/Simulator.h"
#include "../../third_party/nlohmann/json.hpp"
#include "../../third_party/httplib.h"

#include <mutex>
#include <string>
#include <fstream>
#include <iostream>

class HttpServer {
public:
    HttpServer(Simulator& sim, std::mutex& mtx, int port = 8080)
        : sim_(sim), mtx_(mtx), port_(port) {}

    void start() {
        setupRoutes();
        svr_.new_task_queue_depth = 8;
        std::cout << "Server starting on port " << port_ << std::endl;
        svr_.listen("0.0.0.0", port_);
    }

    void stop() { svr_.stop(); }

private:
    Simulator& sim_;
    std::mutex& mtx_;
    int port_;
    httplib::Server svr_;

    void setupRoutes() {
        // 静态文件服务
        svr_.Get("/", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(readFile("frontend/index.html"), "text/html; charset=utf-8");
        });

        svr_.Get("/css/style.css", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(readFile("frontend/css/style.css"), "text/css; charset=utf-8");
        });

        svr_.Get("/js/app.js", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(readFile("frontend/js/app.js"), "application/javascript; charset=utf-8");
        });

        svr_.Get("/js/renderer.js", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(readFile("frontend/js/renderer.js"), "application/javascript; charset=utf-8");
        });

        svr_.Get("/js/heatmap.js", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(readFile("frontend/js/heatmap.js"), "application/javascript; charset=utf-8");
        });

        svr_.Get("/js/api.js", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(readFile("frontend/js/api.js"), "application/javascript; charset=utf-8");
        });

        // API 端点
        svr_.Get("/api/station", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            auto j = JsonSerializer::serializeGraph(sim_.getGraph());
            res.set_content(j.dump(), "application/json");
        });

        svr_.Get("/api/state", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            auto j = sim_.serializeState();
            res.set_content(j.dump(), "application/json");
        });

        svr_.Get("/api/stats", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            nlohmann::json j;
            auto& stats = sim_.getStats();
            j = {
                {"totalGenerated", stats.totalGenerated},
                {"totalCompleted", stats.totalGenerated - stats.currentActive},
                {"currentActive", stats.currentActive},
                {"bottleneckNodeId", stats.bottleneckNodeId},
                {"maxCongestion", stats.maxCongestion}
            };
            res.set_content(j.dump(), "application/json");
        });

        svr_.Get("/api/heatmap", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            HeatMap heatmap;
            heatmap.compute(sim_.getGraph());
            nlohmann::json j;
            j["width"] = heatmap.getWidth();
            j["height"] = heatmap.getHeight();
            j["data"] = nlohmann::json::array();
            auto& grid = heatmap.getGrid();
            for (int y = 0; y < heatmap.getHeight(); y++) {
                auto row = nlohmann::json::array();
                for (int x = 0; x < heatmap.getWidth(); x++) {
                    row.push_back(grid[y][x]);
                }
                j["data"].push_back(row);
            }
            res.set_content(j.dump(), "application/json");
        });

        svr_.Get("/api/multiobjective", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            int from = std::stoi(req.get_param_value("from"));
            int to = std::stoi(req.get_param_value("to"));
            auto j = sim_.serializeMultiObjective(from, to);
            res.set_content(j.dump(), "application/json");
        });

        svr_.Post("/api/start", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            sim_.start();
            res.set_content("{\"status\":\"started\"}", "application/json");
        });

        svr_.Post("/api/pause", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            sim_.pause();
            res.set_content("{\"status\":\"paused\"}", "application/json");
        });

        svr_.Post("/api/reset", [this](const httplib::Request&, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            sim_.reset();
            res.set_content("{\"status\":\"reset\"}", "application/json");
        });

        svr_.Post("/api/speed", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            try {
                auto body = nlohmann::json::parse(req.body);
                int speed = body.value("speed", 1);
                sim_.setSpeed(speed);
                res.set_content("{\"status\":\"ok\",\"speed\":" + std::to_string(speed) + "}", "application/json");
            } catch (...) {
                res.status = 400;
                res.set_content("{\"error\":\"invalid body\"}", "application/json");
            }
        });

        svr_.Post("/api/load", [this](const httplib::Request& req, httplib::Response& res) {
            std::lock_guard<std::mutex> lock(mtx_);
            try {
                auto body = nlohmann::json::parse(req.body);
                std::string path = body.value("path", "data/station_simple.json");
                bool ok = sim_.loadStation(path);
                res.set_content(ok ? "{\"status\":\"loaded\"}" : "{\"status\":\"error\"}", "application/json");
            } catch (...) {
                res.status = 400;
                res.set_content("{\"error\":\"invalid body\"}", "application/json");
            }
        });
    }

    static std::string readFile(const std::string& relativePath) {
        std::string fullPath = "data_project/" + relativePath;
        std::ifstream f(fullPath);
        if (!f.is_open()) {
            f = std::ifstream(relativePath);
        }
        if (!f.is_open()) {
            return "<html><body><h1>File not found: " + relativePath + "</h1></body></html>";
        }
        return std::string(std::istreambuf_iterator<char>(f), {});
    }
};
