// todo docs improvements

#pragma once

// Custom Imports
#include "../../CommonConfig.hpp"
// #include "./NetTestHandler.hpp"
#include "../Net-Link/Packet.hpp"

// Library Imports
#include <etl/memory_model.h>
#include <etl/queue.h>

/**
 * @brief A singleton class for handling packets
 * @author Noah (@BobSaidHi)
 */
class PacketHandler {
  private:
    // static NetTestHandler *netTestHandler;

    // Static pointer to the Singleton instance
    static PacketHandler *instancePtr;

    // Mutex to ensure thread safety
    static std::mutex mtx;

    // Constructors
    PacketHandler() = default;
    ~PacketHandler() = default;

    static_assert(WTbNetConfig::RECV_QUEUE_LEN > 0,
                  "RECV_QUEUE_LEN must be greater than 0");
    static_assert(WTbNetConfig::RECV_QUEUE_LEN <= 255,
                  "RECV_QUEUE_LEN must be less than or equal to 255");
    static etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
                      etl::memory_model::MEMORY_MODEL_SMALL>
        prioritycommandQueue;
    static etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
                      etl::memory_model::MEMORY_MODEL_SMALL>
        commandQueue;
    static etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
                      etl::memory_model::MEMORY_MODEL_SMALL>
        logQueue;
    static etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
                      etl::memory_model::MEMORY_MODEL_SMALL>
        priorityDataQueue;
    static etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
                      etl::memory_model::MEMORY_MODEL_SMALL>
        dataQueue;
    static etl::queue<Packet, WTbNetConfig::RECV_QUEUE_LEN,
                      etl::memory_model::MEMORY_MODEL_SMALL>
        lowPriorityDataQueue;

  public:
    /**
     * @brief Static method to get the singleton instance
     * @returns A pointer to the singleton instance
     */
    static PacketHandler *getInstance();

    /**
     * @brief Check if there is a priority command available
     * @returns true if there is a priority command available, false otherwise
     */
    static bool isPriorityCommandAvailable();

    /**
     * @brief Check if there is a command available
     * @returns true if there is a command available, false otherwise
     */
    static bool isCommandAvailable();

    /**
     * @brief Check if there is a log available
     * @returns true if there is a log available, false otherwise
     */
    static bool isLogAvailable();

    /**
     * @brief Check if there is a priority data available
     * @returns true if there is a priority data available, false otherwise
     */
    static bool isPriorityDataAvailable();

    /**
     * @brief Check if there is data available
     * @returns true if there is data available, false otherwise
     */
    static bool isDataAvailable();

    /**
     * @brief Check if there is low priority data available
     * @returns true if there is low priority data available, false otherwise
     */
    static bool isLowPriorityDataAvailable();

    /**
     * @brief Get the next priority command
     * Pops and returns the next priority command
     * @returns the next priority command
     */
    static Packet getNextPriorityCommand();

    /**
     * @brief Get the next command
     * Pops and returns the next command
     * @returns the next command
     */
    static Packet getNextCommand();

    /**
     * @brief Get the next log
     * Pops and returns the next log
     * @returns the next log
     */
    static Packet getNextLog();

    /**
     * @brief Get the next priority data
     * Pops and returns the next priority data
     * @returns the next priority data
     */
    static Packet getNextPriorityData();

    /**
     * @brief Get the next data
     * Pops and returns the next data
     * @returns the next data
     */
    static Packet getNextData();

    /**
     * @brief Get the next low priority data
     * Pops and returns the next low priority data
     * @returns the next low priority data
     */
    static Packet getNextLowPriorityData();

    /**
     * @brief Process inbound data
     * @param data the data to process
     * @param bytesValid the number of valid bytes in the data array
     * @returns true if the data was processed successfully, false otherwise
     */
    static bool processInboundData(
        etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
        const etl::array<uint8_t, 6> MACAddress, uint8_t bytesValid);
};
