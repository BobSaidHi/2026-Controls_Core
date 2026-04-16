// todo docs improvements

// Include Header
#include "PacketHandler.hpp"

// Custom Imports
#include "..//NetAdapter_A.hpp"

// Logger Config
#define COMPONENT_ID "C.NP.PH" // Config

// Singleton Enforcement Variables
PacketHandler *PacketHandler::instancePtr = nullptr;
std::mutex PacketHandler::mtx;

// Definition of other static member variables
// NetTestHandler *PacketHandler::netTestHandler = new NetTestHandler();
etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
           etl::memory_model::MEMORY_MODEL_SMALL>
    PacketHandler::prioritycommandQueue;
etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
           etl::memory_model::MEMORY_MODEL_SMALL>
    PacketHandler::commandQueue;
etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
           etl::memory_model::MEMORY_MODEL_SMALL>
    PacketHandler::logQueue;
etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
           etl::memory_model::MEMORY_MODEL_SMALL>
    PacketHandler::priorityDataQueue;
etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
           etl::memory_model::MEMORY_MODEL_SMALL>
    PacketHandler::dataQueue;
etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
           etl::memory_model::MEMORY_MODEL_SMALL>
    PacketHandler::lowPriorityDataQueue;

PacketHandler *PacketHandler::getInstance() {
    // Check if the instance exists
    if (instancePtr == nullptr) {
        // Singleton bookkeeping
        std::lock_guard lock(mtx);
        instancePtr = new PacketHandler();
    }
    return instancePtr;
}

bool PacketHandler::isPriorityCommandAvailable() {
    return !prioritycommandQueue.empty();
}

bool PacketHandler::isCommandAvailable() { return !commandQueue.empty(); }

bool PacketHandler::isLogAvailable() { return !logQueue.empty(); }

bool PacketHandler::isPriorityDataAvailable() {
    return !priorityDataQueue.empty();
}

bool PacketHandler::isDataAvailable() { return !dataQueue.empty(); }

bool PacketHandler::isLowPriorityDataAvailable() {
    return !lowPriorityDataQueue.empty();
}

Packet PacketHandler::getNextPriorityCommand() {
    if (prioritycommandQueue.empty()) {
        return Packet();
    } else {
        Packet packet = prioritycommandQueue.front();
        prioritycommandQueue.pop();
        return packet;
    }
}

Packet PacketHandler::getNextCommand() {
    if (commandQueue.empty()) {
        return Packet();
    } else {
        Packet packet = commandQueue.front();
        commandQueue.pop();
        return packet;
    }
}

Packet PacketHandler::getNextLog() {
    if (logQueue.empty()) {
        return Packet();
    } else {
        Packet packet = logQueue.front();
        logQueue.pop();
        return packet;
    }
}

Packet PacketHandler::getNextPriorityData() {
    if (priorityDataQueue.empty()) {
        return Packet();
    } else {
        Packet packet = priorityDataQueue.front();
        priorityDataQueue.pop();
        return packet;
    }
}

Packet PacketHandler::getNextData() {
    if (dataQueue.empty()) {
        return Packet();
    } else {
        Packet packet = dataQueue.front();
        dataQueue.pop();
        return packet;
    }
}

Packet PacketHandler::getNextLowPriorityData() {
    if (lowPriorityDataQueue.empty()) {
        return Packet();
    } else {
        Packet packet = lowPriorityDataQueue.front();
        lowPriorityDataQueue.pop();
        return packet;
    }
}

// bool PacketHandler::processInboundData(
//     etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
//     const etl::array<uint8_t, 6> MACAddress, uint8_t bytesValid) {
//     // Get version ID
//     // version eg: 001X XXXX -> 001  (Shift 5)
//     // constexpr uint8_t version = 0b00100000 >> 5;
//     uint8_t version = data[0] >> 5;

//     // Get the handle ID
//     uint8_t handleID = data[0] & 0b00011111;

//     // Check version
//     if (version == 0) {
//         // Check handle ID
//         // if(handleID == 0x00) {}
//         if (handleID == Packet::PacketType::SpeedTest ||
//             handleID == Packet::PacketType::NetTest) {
//             netTestHandler->processInBoundTest(Packet(data, bytesValid));
//         }

//         // MessagePack and ProtoBuf
//         else if (handleID == Packet::PacketType::MsgPck_PriorityCmd ||
//                  handleID == Packet::PacketType::PBuf_PriorityCmd) {
//             if (prioritycommandQueue.full()) {
//                 logger->critical(FullyQualifiedComponentID,
//                                  "Priority Cmd Queue Full");
//             } else {
//                 prioritycommandQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_Cmd ||
//                    handleID == Packet::PacketType::PBuf_Cmd) {
//             if (commandQueue.full()) {
//                 logger->critical(FullyQualifiedComponentID, "Cmd Queue Full");
//             } else {
//                 commandQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_Log ||
//                    handleID == Packet::PacketType::PBuf_Log) {
//             if (logQueue.full()) {
//                 logger->warn(FullyQualifiedComponentID, "Log Queue Full");
//             } else {
//                 logQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_PriorityData ||
//                    handleID == Packet::PacketType::PBuf_PriorityData) {
//             if (priorityDataQueue.full()) {
//                 logger->critical(FullyQualifiedComponentID,
//                                  "Priority Data Queue Full");
//             } else {
//                 priorityDataQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_Data ||
//                    handleID == Packet::PacketType::PBuf_Data) {
//             if (dataQueue.full()) {
//                 logger->error(FullyQualifiedComponentID, "Data Queue Full");
//                 return false;
//             } else {
//                 dataQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_LowPriorityData ||
//                    handleID == Packet::PacketType::PBuf_LowPriorityData) {
//             if (lowPriorityDataQueue.full()) {
//                 logger->warn(FullyQualifiedComponentID,
//                              "Low Priority Data Queue Full");
//             } else {
//                 lowPriorityDataQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_PriorityCmd ||
//                    handleID == Packet::PacketType::PBuf_PriorityCmd) {
//             if (prioritycommandQueue.full()) {
//                 logger->critical(FullyQualifiedComponentID,
//                                  "Priority Cmd Queue Full");
//             } else {
//                 prioritycommandQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_Cmd ||
//                    handleID == Packet::PacketType::PBuf_Cmd) {
//             if (commandQueue.full()) {
//                 logger->critical(FullyQualifiedComponentID, "Cmd Queue Full");
//             } else {
//                 commandQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_Log ||
//                    handleID == Packet::PacketType::PBuf_Log) {
//             if (logQueue.full()) {
//                 logger->warn(FullyQualifiedComponentID, "Log Queue Full");
//             } else {
//                 logQueue.push(Packet(
//                     data, bytesValid)); // todo - convert to resource pool
//             }
//         } else if (handleID == Packet::PacketType::MsgPck_PriorityData ||
//                    handleID == Packet::PacketType::PBuf_PriorityData) {
//         } else if (handleID == Packet::PacketType::MsgPck_Data ||
//                    handleID == Packet::PacketType::PBuf_Data) {
//         } else if (handleID == Packet::PacketType::MsgPck_LowPriorityData ||
//                    handleID == Packet::PacketType::PBuf_LowPriorityData) {
//         }

//         // Invalid handleID
//         else {
//             logger->error(FullyQualifiedComponentID,
//                           "Received packet with RESERVED handle ID from: ",
//                           NetAdapter_A::formatMACAddress(MACAddress).c_str());
//             return false;
//         }
//     }

//     // Invalid version
//     else {
//         logger->error(FullyQualifiedComponentID,
//                       "Received packet with invalid version from: ",
//                       NetAdapter_A::formatMACAddress(MACAddress).c_str());
//         return false;
//     }

//     // Everything valid
//     return true;
// } // todo