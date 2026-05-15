#pragma once
#include <string>

enum class NodeType {
    ENTRANCE,
    TICKET,
    SECURITY_CHECK,
    TURNSTILE,
    CORRIDOR,
    STAIRS,
    ESCALATOR,
    PLATFORM,
    STATION_EXIT,
    HALL,
    TRANSFER
};

inline std::string nodeTypeToString(NodeType t) {
    switch (t) {
        case NodeType::ENTRANCE:      return "entrance";
        case NodeType::TICKET:        return "ticket";
        case NodeType::SECURITY_CHECK:return "security";
        case NodeType::TURNSTILE:     return "gate";
        case NodeType::CORRIDOR:      return "corridor";
        case NodeType::STAIRS:        return "stairs";
        case NodeType::ESCALATOR:     return "escalator";
        case NodeType::PLATFORM:      return "platform";
        case NodeType::STATION_EXIT:  return "exit";
        case NodeType::HALL:          return "hall";
        case NodeType::TRANSFER:      return "transfer";
    }
    return "unknown";
}

inline NodeType stringToNodeType(const std::string& s) {
    if (s == "entrance")   return NodeType::ENTRANCE;
    if (s == "ticket")     return NodeType::TICKET;
    if (s == "security")   return NodeType::SECURITY_CHECK;
    if (s == "gate")       return NodeType::TURNSTILE;
    if (s == "corridor")   return NodeType::CORRIDOR;
    if (s == "stairs")     return NodeType::STAIRS;
    if (s == "escalator")  return NodeType::ESCALATOR;
    if (s == "platform")   return NodeType::PLATFORM;
    if (s == "exit")       return NodeType::STATION_EXIT;
    if (s == "hall")       return NodeType::HALL;
    if (s == "transfer")   return NodeType::TRANSFER;
    return NodeType::CORRIDOR;
}

enum class PassengerState {
    ENTERING,
    BUYING_TICKET,
    SECURITY_CHECK,
    AT_GATE,
    HEADING_TO_PLATFORM,
    WAITING_TRAIN,
    BOARDING,
    EXITING,
    DONE
};

inline std::string passengerStateToString(PassengerState s) {
    switch (s) {
        case PassengerState::ENTERING:            return "entering";
        case PassengerState::BUYING_TICKET:       return "buying_ticket";
        case PassengerState::SECURITY_CHECK:      return "security_check";
        case PassengerState::AT_GATE:             return "at_gate";
        case PassengerState::HEADING_TO_PLATFORM: return "heading_to_platform";
        case PassengerState::WAITING_TRAIN:       return "waiting_train";
        case PassengerState::BOARDING:            return "boarding";
        case PassengerState::EXITING:             return "exiting";
        case PassengerState::DONE:                return "done";
    }
    return "unknown";
}

struct Position {
    double x = 0;
    double y = 0;
    int floor = 0;
};
