#pragma once

// Custom Imports
#include "../CommonConfig.hpp"

// Libraries Imports
#include <cstdint>
#include <etl/array.h>
#include <etl/format_spec.h>
#include <etl/string.h>
#include <etl/to_string.h>

/* Constants */

// constexpr uint8_t MAC_ADDR_STR_LEN =
//     sizeof("00:00:00:00:00:00"); // I can't count --Noah

// TODO: Receive support

// TODO: Convert to interface?, split other stuff into MACAdapter or something?

/**
 * @brief Abstract Base Class that represents the basic functions and data types
 * required for a Network Adapter
 */
class NetAdapter_A {
  protected:
    /* MARK: Protected Member Variables */

    // etl::array<uint8_t, 6> MACAddress;
    // const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> toSend;

  public: // MARK: Public
    static constexpr char *TAG = "NA";

    NetAdapter_A();

    /* Public Virtual Utility Functions*/

    /**
     * @brief Get the MAC address of the network interface
     * @returns the MAC address of the network interface
     */
    // virtual etl::array<uint8_t, 6> getMACAddress() const = 0;

    /* Public Static Concrete Utility Functions */

    /**
     * @brief Format a MAC address as a string
     * @param rawMACAddress the MAC address to format
     * @returns the formatted MAC address
     */
    // static etl::string<MAC_ADDR_STR_LEN>
    // formatMACAddress(const etl::array<uint8_t, 6> rawMACAddress);

    /**
     * @brief Test the throughput of the network
     * @param maxTime the maximum time to run the test
     * @param maxBytes the maximum number of bytes to send, will be rounded up
     * to the nearest multiple of WTbNetConfig::MAX_PACKET_ABS_LEN
     * @param checkLatency whether to check the latency of the network
     * @param verifyReceipt whether to verify the data was received (ACK)
     * @returns the throughput in bits per second
     */
    uint32_t testThroughput(uint_fast8_t maxTime = 60,
                            uint_fast32_t maxBytes = 1000,
                            /*NetAdapter_A /*adapterInstance,*/
                            bool checkLatency = false,
                            bool verifyReceipt = true);

    /**
     * @brief Send data
     *
     * @param dest_MAC_addr the MAC address of the destination
     * @param data the data to send
     * @param bytesValid the number of valid bytes in the data array
     * @param verifyReceipt whether to verify the data was received
     */
    virtual bool
    send(const etl::array<uint8_t, 6> destMAC_addr,
         const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
         const uint_fast8_t bytesValid, const bool verifyReceipt) = 0;

    /**
     * @brief Send data to all devices
     *
     * @param data the data to send
     * @param bytesValid the number of valid bytes in the data array
     * @param verifyReceipt whether to verify the data was received
     */
    virtual bool
    sendAll(const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
            const uint_fast8_t bytesValid) = 0;
};
