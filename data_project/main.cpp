#include "include/core/Config.h"
#include "include/simulation/Simulator.h"
#include "include/io/JsonSerializer.h"
#include "include/io/StationLoader.h"
#include "include/analysis/HeatMap.h"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <csignal>
#include <atomic>

#include "third_party/nlohmann/json.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

static Simulator g_sim;
static std::mutex g_simMutex;
static std::atomic<bool> g_running{true};

static std::string readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return "";
    return std::string(std::istreambuf_iterator<char>(f), {});
}

static std::string getFrontendFile(const std::string& name) {
    // Try multiple possible base paths
    static const char* bases[] = {
        "",
        "data_project/",
        "../data_project/",
        "../../data_project/"
    };
    for (auto base : bases) {
        std::string fullPath = std::string(base) + name;
        std::string content = readFile(fullPath);
        if (!content.empty()) {
            std::cerr << "[HTTP] Served: " << fullPath << " (" << content.size() << " bytes)" << std::endl;
            return content;
        }
    }
    std::cerr << "[HTTP] File not found: " << name << std::endl;
    return "";
}

class HttpServer {
public:
    HttpServer(int port) : port_(port), listenSock_(INVALID_SOCKET) {}

    bool start() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "[HTTP] WSAStartup failed" << std::endl;
            return false;
        }

        listenSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSock_ == INVALID_SOCKET) {
            std::cerr << "[HTTP] socket() failed" << std::endl;
            return false;
        }

        int opt = 1;
        setsockopt(listenSock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons((u_short)port_);

        if (bind(listenSock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "[HTTP] bind() failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        if (listen(listenSock_, 10) == SOCKET_ERROR) {
            std::cerr << "[HTTP] listen() failed" << std::endl;
            return false;
        }

        // Start listener thread
        listenerThread_ = std::thread(&HttpServer::listenerLoop, this);
        listenerThread_.detach();

        return true;
    }

    void stop() {
        g_running = false;
        if (listenSock_ != INVALID_SOCKET) {
            closesocket(listenSock_);
            listenSock_ = INVALID_SOCKET;
        }
        WSACleanup();
    }

private:
    int port_;
    SOCKET listenSock_;
    std::thread listenerThread_;

    void listenerLoop() {
        std::cerr << "[HTTP] Listener thread started on port " << port_ << std::endl;

        while (g_running) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(listenSock_, &readfds);

            timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 50000; // 50ms

            int ret = select(0, &readfds, nullptr, nullptr, &tv);
            if (ret > 0 && FD_ISSET(listenSock_, &readfds)) {
                sockaddr_in clientAddr;
                int addrLen = sizeof(clientAddr);
                SOCKET clientSock = accept(listenSock_, (sockaddr*)&clientAddr, &addrLen);
                if (clientSock != INVALID_SOCKET) {
                    handleClient(clientSock);
                }
            }
        }
        std::cerr << "[HTTP] Listener thread stopped" << std::endl;
    }

    void handleClient(SOCKET sock) {
        // Wait for data with timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        char buf[8192] = {};
        int totalReceived = 0;

        // Receive with small wait
        int selRet = select(0, &readfds, nullptr, nullptr, &tv);
        if (selRet > 0) {
            totalReceived = recv(sock, buf, sizeof(buf) - 1, 0);
        }

        if (totalReceived <= 0) {
            std::cerr << "[HTTP] recv failed or empty: " << totalReceived << std::endl;
            sendErrorResponse(sock, 400, "Bad Request");
            closesocket(sock);
            return;
        }

        buf[totalReceived] = '\0';

        // Parse method and path
        std::string request(buf, totalReceived);
        size_t space1 = request.find(' ');
        size_t space2 = (space1 != std::string::npos) ? request.find(' ', space1 + 1) : std::string::npos;

        if (space1 == std::string::npos || space2 == std::string::npos) {
            sendErrorResponse(sock, 400, "Bad Request");
            closesocket(sock);
            return;
        }

        std::string method = request.substr(0, space1);
        std::string path = request.substr(space1 + 1, space2 - space1 - 1);

        std::string response;
        std::string contentType = "application/json";

        {
            std::lock_guard<std::mutex> lock(g_simMutex);
            response = routeRequest(method, path, request, contentType);
        }

        sendResponse(sock, 200, contentType, response);
        closesocket(sock);
    }

    std::string routeRequest(const std::string& method, const std::string& path,
                              const std::string& request, std::string& contentType) {
        // Static files
        if (path == "/" || path == "/index.html") {
            contentType = "text/html; charset=utf-8";
            return getFrontendFile("frontend/index.html");
        }
        if (path == "/css/style.css") {
            contentType = "text/css; charset=utf-8";
            return getFrontendFile("frontend/css/style.css");
        }
        if (path == "/js/app.js") {
            contentType = "application/javascript; charset=utf-8";
            return getFrontendFile("frontend/js/app.js");
        }
        if (path == "/js/renderer.js") {
            contentType = "application/javascript; charset=utf-8";
            return getFrontendFile("frontend/js/renderer.js");
        }
        if (path == "/js/heatmap.js") {
            contentType = "application/javascript; charset=utf-8";
            return getFrontendFile("frontend/js/heatmap.js");
        }
        if (path == "/js/api.js") {
            contentType = "application/javascript; charset=utf-8";
            return getFrontendFile("frontend/js/api.js");
        }

        // API endpoints
        if (path == "/api/station") {
            return JsonSerializer::serializeGraph(g_sim.getGraph());
        }
        if (path == "/api/state") {
            return g_sim.serializeState();
        }
        if (path == "/api/stats") {
            auto& s = g_sim.getStats();
            return "{\"totalGenerated\":" + std::to_string(s.totalGenerated) +
                ",\"totalCompleted\":" + std::to_string(s.totalGenerated - s.currentActive) +
                ",\"currentActive\":" + std::to_string(s.currentActive) +
                ",\"bottleneckNodeId\":" + std::to_string(s.bottleneckNodeId) +
                ",\"maxCongestion\":" + std::to_string(s.maxCongestion) + "}";
        }
        if (path == "/api/start") {
            g_sim.start();
            return "{\"status\":\"started\"}";
        }
        if (path == "/api/pause") {
            g_sim.pause();
            return "{\"status\":\"paused\"}";
        }
        if (path == "/api/reset") {
            g_sim.reset();
            return "{\"status\":\"reset\"}";
        }
        if (path == "/api/speed") {
            size_t bodyStart = request.find("\r\n\r\n");
            if (bodyStart != std::string::npos) {
                std::string body = request.substr(bodyStart + 4);
                try {
                    auto j = nlohmann::json::parse(body, nullptr, false);
                    if (!j.is_null() && j.contains("speed")) {
                        g_sim.setSpeed(j["speed"].get<int>());
                    }
                } catch (...) {}
            }
            return "{\"status\":\"ok\"}";
        }

        return "{\"error\":\"not found\",\"path\":\"" + path + "\"}";
    }

    void sendResponse(SOCKET sock, int code, const std::string& contentType, const std::string& body) {
        std::string statusText = (code == 200) ? "OK" : "Error";
        std::string resp = "HTTP/1.1 " + std::to_string(code) + " " + statusText + "\r\n"
            "Content-Type: " + contentType + "\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;

        int total = (int)resp.size();
        int sent = 0;
        while (sent < total) {
            int n = send(sock, resp.c_str() + sent, total - sent, 0);
            if (n <= 0) break;
            sent += n;
        }
    }

    void sendErrorResponse(SOCKET sock, int code, const std::string& msg) {
        sendResponse(sock, code, "text/plain", msg);
    }
};

// StationLoader implementation
bool StationLoader::parseStationJson(const std::string& content, Graph& graph) {
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(content);
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }

    if (!j.contains("nodes") || !j.contains("edges")) {
        std::cerr << "Missing 'nodes' or 'edges' in station JSON" << std::endl;
        return false;
    }

    for (const auto& nj : j["nodes"]) {
        Node node;
        node.id = nj.value("id", -1);
        node.type = stringToNodeType(nj.value("type", "corridor"));
        node.pos.x = nj.value("x", 0.0);
        node.pos.y = nj.value("y", 0.0);
        node.pos.floor = nj.value("floor", 0);
        node.name = nj.value("name", "");
        node.capacity = nj.value("capacity", 100.0);
        node.freeFlowSpeed = nj.value("speed", 1.5);
        node.area = nj.value("area", 20.0);
        graph.addNode(node);
    }

    for (const auto& ej : j["edges"]) {
        Edge edge;
        edge.from = ej.value("from", -1);
        edge.to = ej.value("to", -1);
        edge.length = ej.value("length", 1.0);
        edge.width = ej.value("width", 2.0);
        graph.addEdge(edge);
    }

    return true;
}

bool Simulator::loadStation(const std::string& path) {
    return StationLoader::loadFromFile(path, graph_);
}

void signalHandler(int) {
    g_running = false;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Config config;
    std::string stationPath = "data/station_simple.json";

    if (argc > 1) {
        stationPath = argv[1];
    }

    std::cout << "============================================" << std::endl;
    std::cout << "   Subway Crowd Flow Simulation System" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "Loading station: " << stationPath << std::endl;

    if (!g_sim.loadStation(stationPath)) {
        std::cerr << "Failed to load station file!" << std::endl;
        return 1;
    }

    Graph& graph = g_sim.getGraph();
    std::cout << "Station loaded: " << graph.nodeCount() << " nodes, "
              << graph.edgeCount() << " edges" << std::endl;

    HttpServer server(config.serverPort);
    if (!server.start()) {
        std::cerr << "Failed to start HTTP server!" << std::endl;
        return 1;
    }

    std::cout << "Open http://localhost:" << config.serverPort << " in your browser" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    // Simulation loop runs in main thread
    while (g_running) {
        {
            std::lock_guard<std::mutex> lock(g_simMutex);
            if (g_sim.isRunning()) {
                for (int i = 0; i < g_sim.getSpeedMultiplier(); i++) {
                    g_sim.step();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    server.stop();
    std::cout << "\nShutting down." << std::endl;
    return 0;
}
