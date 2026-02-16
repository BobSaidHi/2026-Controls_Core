// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// Custom Imports
#include "../../CommonConfig.hpp"
#include "Packet.hpp"
// #include "../Net-Link/PacketHandler.hpp"
#include "../Net-Phy/AdapterWLAN.hpp"
#include "../NetAdapter_A.hpp"
#include "AdapterESPNow.hpp"

// Library Imports
#include <WiFi.h>
#include <etl/string.h>

// Initialize Static Member Variables
uint_fast32_t AdapterESPNow::txEvents = 0;
uint_fast32_t AdapterESPNow::bytesSent = 0;
uint_fast32_t AdapterESPNow::bytesNotSent = 0;
uint_fast32_t AdapterESPNow::rxEvents = 0;
uint_fast32_t AdapterESPNow::bytesReceived = 0;

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
    if (esp_now_register_send_cb(AdapterESPNow::OnDataSent) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register send callback");
        return false;
    }
    if (esp_now_register_recv_cb(
            (esp_now_recv_cb_t)AdapterESPNow::OnDataRecv) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register receive callback");
        return false;
    }

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
/**
 * @brief Callback when data is sent
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/v5.5.2/esp32c5/api-reference/network/esp_now.html#send-esp-now-data
 */
void AdapterESPNow::OnDataSent(const esp_now_send_info_t *tx_info,
                               esp_now_send_status_t status) {
    txEvents++;
    if (status == ESP_NOW_SEND_SUCCESS) {
        bytesSent += tx_info->data_len;
    } else {
        bytesNotSent += tx_info->data_len;
    }
}

/**
 * @brief Callback when data is received
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/v5.5.2/esp32c5/api-reference/network/esp_now.html#receiving-esp-now-data
 */
void AdapterESPNow::OnDataRecv(const uint8_t *mac, const uint8_t *incomingData,
                               int len) {
    // todo log Serial.print("Bytes received: ");
    // todo log Serial.println(len);s
    rxEvents++;
    bytesReceived += len;

    // Process MAC Address into a C++ array format
    // etl::array<uint8_t, 6> MACAddress = {0};
    // std::copy(mac, mac + 6, MACAddress.begin());
    // etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> data = {0};
    // std::copy(mac, mac + len, MACAddress.begin()); // todo ???

    // Check te length of the received data
    if (len == 0) {
        ESP_LOGE(TAG, "Received empty packet");
        return;
    }

    // Convert the received data into a C++ Array
    // Apparently the ETL implementation has a handy function to do this
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> incomingDataArray = {
        0};
    incomingDataArray.assign(incomingData, incomingData + len);

    // todo: enqueue to process later
    // todo: Handle the received data
    // PacketHandler::processInboundData(incomingDataArray, MACAddress, len); //
    // todo
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
        ESP_LOGW(TAG, "Send failed");
        return false;
    } else {
        ESP_LOGV(TAG, "Data sent");
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
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        ESP_LOGE(TAG, "Peer registration failed: ",
                 AdapterWLAN::formatMACAddress(MACAddress).c_str());
        return false;
    }
    ESP_LOGD(TAG, "Peer registered: ",
             AdapterWLAN::formatMACAddress(MACAddress).c_str());

    // esp_now_set_peer_rate_config(peerInfo.peer_addr, ...);

    return true;
}

bool AdapterESPNow::deregisterPeer(const etl::array<uint8_t, 6> MACAddress) {
    ESP_LOGE(TAG, "Not implemented");
    return false;
}

// ESP Now Specific
