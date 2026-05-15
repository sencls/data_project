#pragma once

#include "core/Types.h"
#include "core/Node.h"
#include "core/Edge.h"
#include "core/Graph.h"
#include "datastructure/DynamicArray.h"
#include "datastructure/HashMap.h"
#include "simulation/Passenger.h"
#include "pathfinding/Pathfinder.h"

class MovementEngine {
public:
    void advanceAll(HashMap<int, Passenger>& passengers, Graph& graph, int tick) {
        DynamicArray<int> ids = passengers.keys();
        DynamicArray<int> toRemove;

        for (int idx = 0; idx < ids.size(); idx++) {
            int pid = ids[idx];
            Passenger& p = passengers[pid];
            p.totalTravelTime += 1.0;

            bool removeThis = false;

            if (p.state == PassengerState::DONE) {
                toRemove.push_back(pid);
                Node* node = graph.getNodePtrMut(p.currentNodeId);
                if (node && node->currentOccupancy > 0) {
                    node->currentOccupancy--;
                }
                removeThis = true;
            }

            if (!removeThis && !p.hasNextNode()) {
                p.state = PassengerState::DONE;
                toRemove.push_back(pid);
                Node* node = graph.getNodePtrMut(p.currentNodeId);
                if (node && node->currentOccupancy > 0) {
                    node->currentOccupancy--;
                }
                removeThis = true;
            }

            if (!removeThis) {
                int nextNodeId = p.getNextNodeId();
                Node* nextNode = graph.getNodePtrMut(nextNodeId);
                Node* curNode = graph.getNodePtrMut(p.currentNodeId);

                if (nextNode && nextNode->isFull()) {
                    p.waitTime += 1.0;
                    if (p.waitTime > 30.0 * (1.0 - p.patience + 0.1)) {
                        PathResult newPath = Pathfinder::dijkstra(graph, p.currentNodeId, p.destNodeId);
                        if (newPath.found && newPath.path.size() > 1) {
                            p.setPath(newPath.path);
                        }
                    }
                    removeThis = false; // stay in loop, just skip movement
                } else {
                    double edgeLength = 1.0;
                    if (curNode) {
                        DynamicArray<Edge>& edges = graph.getOutEdges(p.currentNodeId);
                        for (int j = 0; j < edges.size(); j++) {
                            if (edges[j].to == nextNodeId) {
                                edgeLength = edges[j].length;
                                break;
                            }
                        }
                    }

                    double speed = 1.5;
                    if (nextNode) {
                        speed = nextNode->getEffectiveSpeed();
                    }
                    if (speed <= 0) speed = 0.1;

                    bool arrived = p.advanceOnEdge(speed, edgeLength);
                    if (arrived) {
                        Node* oldNode = graph.getNodePtrMut(p.currentNodeId);
                        if (oldNode && oldNode->currentOccupancy > 0) {
                            oldNode->currentOccupancy--;
                        }

                        p.moveToNextNode();

                        Node* newNode = graph.getNodePtrMut(p.currentNodeId);
                        if (newNode) {
                            newNode->currentOccupancy++;
                        }

                        updatePassengerState(p, graph);
                    }
                }
            }
        }

        for (int idx = 0; idx < toRemove.size(); idx++) {
            passengers.erase(toRemove[idx]);
        }
    }

private:
    void updatePassengerState(Passenger& p, Graph& graph) {
        Node* node = graph.getNodePtrMut(p.currentNodeId);
        if (!node) return;

        if (node->type == NodeType::ENTRANCE) {
            p.state = PassengerState::ENTERING;
        } else if (node->type == NodeType::TICKET) {
            p.state = PassengerState::BUYING_TICKET;
        } else if (node->type == NodeType::SECURITY_CHECK) {
            p.state = PassengerState::SECURITY_CHECK;
        } else if (node->type == NodeType::TURNSTILE) {
            p.state = PassengerState::AT_GATE;
        } else if (node->type == NodeType::PLATFORM) {
            p.state = PassengerState::WAITING_TRAIN;
        } else if (node->type == NodeType::STATION_EXIT) {
            p.state = PassengerState::DONE;
        } else {
            p.state = PassengerState::HEADING_TO_PLATFORM;
        }
    }
};
