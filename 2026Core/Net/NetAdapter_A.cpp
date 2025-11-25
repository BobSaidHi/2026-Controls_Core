// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// Import Header
#include "NetAdapter_A.hpp"

// Custom Imports
#include "Net-Link/Packet.hpp"

// Library Imports
#include "esp_log.h"

// Logging Config

// Definition of static member variables

/* MARK: Standard Functions */

NetAdapter_A::NetAdapter_A() { // TODO: consider making this static
    // Initialize Logger
    ESP_LOGI(TAG, "init");
}

/* Public Static Concrete Utility Functions */
// MARK: Utilities

// etl::string<MAC_ADDR_STR_LEN>
// NetAdapter_A::formatMACAddress(const etl::array<uint8_t, 6> rawMACAddress) {
//     // Serial.println("formatMACAddress called");
//     // delay(1500);
//     LOG_TRACE(FullyQualifiedComponentID, "func. called");
//     // delay(1500);
//     //   Format the MAC address and log it
//     etl::string<MAC_ADDR_STR_LEN> MACAddressFormatted = "";
//     LOG_TRACE(FullyQualifiedComponentID, "Created formatted string");
//     // delay(1500);

//     constexpr uint_fast8_t segmentDigits = 2;
//     etl::format_spec format;
//     format.hex();
//     format.width(segmentDigits);
//     format.fill('0');
//     LOG_TRACE(FullyQualifiedComponentID, "Setup format spec");
//     // delay(1500);

//     LOG_TRACE(FullyQualifiedComponentID, "Formatting");
//     // delay(1500);
//     etl::string<segmentDigits> formatBufferTmp;
//     MACAddressFormatted.append(
//         etl::to_string(rawMACAddress.at(0), formatBufferTmp, format));
//     MACAddressFormatted.append(":");
//     MACAddressFormatted.append(
//         etl::to_string(rawMACAddress.at(1), formatBufferTmp, format));
//     MACAddressFormatted.append(":");
//     MACAddressFormatted.append(
//         etl::to_string(rawMACAddress.at(2), formatBufferTmp, format));
//     MACAddressFormatted.append(":");
//     MACAddressFormatted.append(
//         etl::to_string(rawMACAddress.at(3), formatBufferTmp, format));
//     MACAddressFormatted.append(":");
//     MACAddressFormatted.append(
//         etl::to_string(rawMACAddress.at(4), formatBufferTmp, format));
//     MACAddressFormatted.append(":");
//     MACAddressFormatted.append(
//         etl::to_string(rawMACAddress.at(5), formatBufferTmp, format));

//     LOG_TRACE(FullyQualifiedComponentID, "ret");
//     return MACAddressFormatted;
// }

// uint32_t NetAdapter_A::testThroughput(uint_fast8_t maxTime, uint_fast32_t maxBytes,
//                                       bool checkLatency, bool verifyReceipt) {
//     LOG_TRACE(FullyQualifiedComponentID, "Checking WiFi");
//     // delay(1000);
//     //  Verify WiFi is connected
//     //  assert(WiFi.status() == WL_CONNECTED); // todo superclass or include

//     // Create a Packet
//     Packet packet;
//     etl::array<uint8_t, WTbNetConfig::MAX_PACKET_LENGTH> data;
//     // Prepare data
//     for (int i = 1; i < WTbNetConfig::MAX_PACKET_LENGTH; i++) {
//         data[i] = (uint8_t)i;
//     }
//     LOG_TRACE(FullyQualifiedComponentID, "Default Packet Ready");
//     // delay(1000);

//     // Setup Packet if not checking latency
//     if (!checkLatency) {
//         LOG_TRACE(FullyQualifiedComponentID, "Encoding Latency Info");

//         static_assert(data.size() == WTbNetConfig::MAX_PACKET_LENGTH);

//         packet.setContents(data, data.size(), Packet::PacketType::SpeedTest);
//     }
//     LOG_TRACE(FullyQualifiedComponentID, "Packet Fully Ready");
//     // delay(1000);

//     // Calculate maximum test duration
//     const uint64_t startTime = millis();
//     LOG_DEBUG_DATA(FullyQualifiedComponentID, "sT: ", startTime);
//     const uint64_t endTime = millis() + ((uint32_t)maxTime * 1000);
//     LOG_DEBUG_DATA(FullyQualifiedComponentID, "eT: ", endTime);
//     LOG_TRACE(FullyQualifiedComponentID, "Test Duration Calculated");
//     // delay(1000);

//     // Track number of bytes actually sent
//     uint32_t bytesSent = 0;

//     // Send packets until the test is over
//     while (millis() < endTime && bytesSent < maxBytes) {
//         LOG_TRACE(FullyQualifiedComponentID, "SP");
//         // Update packet if checking latency
//         // constexpr uint8_t millisLen = sizeof(millis());
//         data[0] = (uint8_t)false;
//         if (checkLatency) {
//             LOG_DEBUG(FullyQualifiedComponentID, "Checking Latency");
//             auto *currentTime = (uint8_t *)millis();
//             static_assert(data.size() == WTbNetConfig::MAX_PACKET_LENGTH);
//             LOG_TRACE(FullyQualifiedComponentID, "Copying Time");
//             memcpy(data.begin() + 1, &currentTime, sizeof(currentTime));
//             LOG_TRACE(FullyQualifiedComponentID, "Time Copied");
//             packet.setContents(data, data.size(), Packet::PacketType::NetTest);
//         }
//         LOG_TRACE(FullyQualifiedComponentID, "Packet Ready");

//         // Actually send the packet
//         this->sendAll(packet.getRawPacket(), packet.getLengthValid());
//         // Statistics
//         bytesSent += WTbNetConfig::MAX_PACKET_ABS_LEN;
//     }
//     LOG_TRACE(FullyQualifiedComponentID, "Test Almost Complete");

//     // Send once more w/ end indicator
//     // Update packet if checking latency
//     // constexpr uint8_t millisLen = sizeof(millis());
//     data[0] = (uint8_t)true;
//     if (checkLatency) {
//         LOG_DEBUG(FullyQualifiedComponentID, "Checking Latency");
//         auto *currentTime = (uint8_t *)millis();
//         static_assert(data.size() == WTbNetConfig::MAX_PACKET_LENGTH);
//         LOG_TRACE(FullyQualifiedComponentID, "Copying Time");
//         memcpy(data.begin() + 1, &currentTime, sizeof(currentTime));
//         LOG_TRACE(FullyQualifiedComponentID, "Time Copied");
//         packet.setContents(data, data.size(), Packet::PacketType::NetTest);
//     }

//     // Actually send the packet
//     this->sendAll(packet.getRawPacket(), packet.getLengthValid());
//     // Statistics
//     bytesSent += WTbNetConfig::MAX_PACKET_ABS_LEN;
//     LOG_TRACE(FullyQualifiedComponentID, "Test Complete");

//     // Compute actual speeds
//     LOG_DEBUG_DATA(FullyQualifiedComponentID, "bytesSent: ", bytesSent);
//     uint64_t elapsedTime = millis() - startTime;
//     LOG_DEBUG_DATA(FullyQualifiedComponentID, "timeElapsed: ", elapsedTime);
//     auto netSpeed = (uint32_t)((uint64_t)bytesSent * 1000 / elapsedTime);
//     logger->info(FullyQualifiedComponentID, "SPD-bps: ", netSpeed);
//     LOG_TRACE(FullyQualifiedComponentID, "ret");
//     return netSpeed;
// }
