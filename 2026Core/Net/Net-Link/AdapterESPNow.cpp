// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// Custom Imports
#include "../../CommonConfig.hpp"
#include "Packet.hpp"
// #include "../Net-Link/PacketHandler.hpp"
#include "../NetAdapter_A.hpp"
#include "AdapterESPNow.hpp"

// Library Imports
#include <WiFi.h>
#include <etl/string.h>

/* Standard Functions */
// MARK: Standard

AdapterESPNow::AdapterESPNow() { ESP_LOGI(TAG, "init"); }

bool AdapterESPNow::begin() {
    // todo check WiFi status
    /*if (WiFi.status() != WL_IDLE_STATUS || WiFi.status() != WL_CONNECTED ||
    WiFi.status() != WL_NO_SSID_AVAIL) {
        ESP_LOGE(TAG, "WiFi not connected");
        return false;
    }*/

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        ESP_LOGE(TAG, "ESP-NOW init. fail");
        return false;
    }
    ESP_LOGD(TAG, "ESP-NOW init. success");

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Transmitted packet
    esp_now_register_send_cb(AdapterESPNow::OnDataSent);
    esp_now_register_recv_cb((esp_now_recv_cb_t)AdapterESPNow::OnDataRecv);

    // todo ? Register peer

    // todo ? Add peer

    return true; // todo
}

// Standard Funcs.

/* Utility Functions */
// MARK: Utility

// etl::array<uint8_t, 6> AdapterESPNow::getMACAddress() const {
//     return localWLANAdapter.getMACAddress();
// }

// Utility Functions

/* Private ESP IDF Helper/ Callback Functions */
// MARK: ESP-IDF Callbacks

// todo - track stats?
void AdapterESPNow::OnDataSent(const esp_now_send_info_t *tx_info,
                               esp_now_send_status_t status) {
    // Log Success (debug) or Failure
    // if (status == ESP_NOW_SEND_SUCCESS &&
    //     WTbCommonConfig::DEBUG_LEVEL > BSILogger::LOG_LEVELS::DEBUG) {
    //     return; // todo: Don't log success unless in debug mode
    // }

    // Convert the old C Style array to a C++ array
    etl::array<uint8_t, 6> MACAddress = {0};
    std::copy(tx_info->des_addr, tx_info->des_addr + 6, MACAddress.begin());

    // Format as a string
    etl::string<AdapterWLAN_A::MAC_ADDR_STR_LEN> formattedMACAddress =
        AdapterWLAN_A::formatMACAddress(MACAddress);

    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGD(TAG, "Data sent to: ", formattedMACAddress.c_str());
    } else {
        ESP_LOGW(TAG, "Send failed to: ", formattedMACAddress.c_str());
    }
}

// Callback when data is received
void AdapterESPNow::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData,
                               int len) {
    // todo log Serial.print("Bytes received: ");
    // todo log Serial.println(len);s
    // todo: stats

    // Process MAC Address into a C++ array format
    etl::array<uint8_t, 6> MACAddress = {0};
    std::copy(mac, mac + 6, MACAddress.begin());
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> data = {0};
    // std::copy(mac, mac + len, MACAddress.begin()); // todo ???

    // Check te length of the received data
    if (len == 0) {
        ESP_LOGE(TAG, "Received empty packet from: ",
                 AdapterWLAN_A::formatMACAddress(MACAddress).c_str());
        return;
    }

    // Convert the received data into a C++ Array
    // Apparently the ETL implementation has a handy function to do this
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> incomingDataArray = {
        0};
    incomingDataArray.assign(incomingData, incomingData + len);

    // todo: Handle the received data
    // PacketHandler::processInboundData(incomingDataArray, MACAddress, len); // todo
}

/* Network Overrides */
// MARK: Network

bool AdapterESPNow::send(
    const etl::array<uint8_t, 6> destMAC_addr,
    const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
    const uint_fast8_t bytesValid, const bool verifyReceipt) {

    esp_err_t result =
        esp_now_send(destMAC_addr.data(), data.data(), bytesValid);

    if (!verifyReceipt) {
        return true;
    }

    if (result != ESP_OK) {
        return false;
    } else {
        return true;
    }
}

bool AdapterESPNow::sendAll(
    const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
    const uint_fast8_t bytesValid) {
    ESP_LOGV(TAG, "sendAll");

    esp_err_t result = esp_now_send(NULL, data.data(), bytesValid);

    if (result != ESP_OK) {
        return false;
    }
    return true;
}

// Net. Overrides

/* ESP Now Wrappers */
// MARK: ESP Now Specific

bool AdapterESPNow::registerPeer(const etl::array<uint8_t, 6> MACAddress) {
    // Construct Peer Information
    esp_now_peer_info_t peerInfo = {};

    // Validate Array Sizes
    static_assert(sizeof(peerInfo.peer_addr) == 6, "MAC address size mismatch");
    static_assert(MACAddress.size() == 6, "MAC address size mismatch");
    memcpy(peerInfo.peer_addr, MACAddress.data(), sizeof(peerInfo.peer_addr));

    // Register Peer
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        ESP_LOGI(TAG, "Peer registered: ",
                 AdapterWLAN_A::formatMACAddress(MACAddress).c_str());
        return true;
    } else {
        ESP_LOGE(TAG, "Peer registration failed: ",
                 AdapterWLAN_A::formatMACAddress(MACAddress).c_str());
        return false;
    }
}

bool AdapterESPNow::deregisterPeer(const etl::array<uint8_t, 6> MACAddress) {
    ESP_LOGE(TAG, "Not implemented");
    return false;
}

// ESP Now Specific
