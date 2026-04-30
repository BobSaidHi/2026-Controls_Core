#pragma once

// Custom Imports
// #include "../Net-Phy/AdapterWLAN.hpp"
#include "../NetAdapter_A.hpp"

// Library Imports
#include <esp_wifi.h>
// Workaround for header mismatch: esp_now.h expects wifi_txe_rate_config_t
typedef wifi_tx_rate_config_t wifi_txe_rate_config_t;
#include <esp_now.h>

/**
 * @brief Network Adapter for the ESPNow protocol and wrapper for ESP-NOW
 * functionality
 * @author Noah (@BobSaidHi)
 */
class AdapterESPNow : public NetAdapter_A {
  private:                         // MARK: Private
    static uint_fast32_t txEvents; // todo: check against last years code
    static uint_fast32_t bytesSent;
    static uint_fast32_t bytesNotSent;
    static uint_fast32_t rxEvents;
    static uint_fast32_t bytesReceived;
    // MARK: ESP-IDF Callbacks

    /**
     * @brief Callback when data is sent
     * @param tx_info Sending information for ESPNOW data
     * @param status status of sending ESPNOW data (succeed or fail). This is
     * will be removed later, since the tx_info->tx_status also works.
     */
    static void OnDataSent(const esp_now_send_info_t *tx_info,
                           esp_now_send_status_t status);

    /**
     * @brief Callback when data is received
     * @param mac pointer to a uint8_t array containing the MAC address of the
     * sender
     * @param incomingData pointer to a uint8_t array containing the incoming
     * data
     * @param len the length of the incoming data
     */
    static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData,
                           int len);

  public: // MARK: Public
    static constexpr const char *TAG = "AE";

    AdapterESPNow();
    ~AdapterESPNow() = default;

    /**
     * @brief Initialize the ESPNow network adapter
     */
    bool begin(); // todo override

    // todo
    // bool begin(const uint8_t wiFiChannel = WTbNetConfig::WIFI_DEFAULT_CH); //
    // @deprecated ? - controlled by WiFi setup?

    // MARK: Network

    /**
     * @brief Send data to a specific MAC address
     * @param destMAC_addr the MAC address of the destination peer
     * @param data the data to send
     * @param bytesValid the number of valid bytes to send in the data array
     * @param verifyReceipt whether to verify the data was received
     * @returns true if the data was sent successfully, false otherwise
     */
    bool send(const etl::array<uint8_t, 6> &destMAC_addr,
              const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
              const uint_fast8_t bytesValid, const bool verifyReceipt) override;

    /**
     * @brief Send data to all peers
     * @param data the data to send
     * @param bytesValid the number of valid bytes to send in the data array
     * @returns true if the data was sent successfully, false otherwise
     */
    bool
    sendAll(const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
            const uint_fast8_t bytesValid) override;

    // MARK: ESP Now Specific

    /**
     * @brief Register a peer
     * @param MACAddress the MAC address of the peer to register
     * @returns true if the peer was registered successfully, false otherwise
     */
    bool registerPeer(const etl::array<uint8_t, 6> &MACAddress);

    /**
     * @brief Deregister a peer
     * @param MACAddress the MAC address of the peer to deregister
     * @returns true if the peer was deregistered successfully, false otherwise
     * @deprecated NotImplemented
     */
    bool deregisterPeer(const etl::array<uint8_t, 6> &MACAddress);

    /**
     * @brief Get the MAC address of the network interface
     * @returns the MAC address of the network interface
     */
    // etl::array<uint8_t, 6> getMACAddress() const override;
};

// ESP_NOW_HPP
