// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "Packet.hpp" //todo
#include "esp_log.h"

/* MARK: Standard Functions */

Packet::Packet() {};

Packet::Packet(etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &bin,
               const uint_fast8_t dataLength) {
    setContentsRaw(bin, dataLength);
}

Packet::Packet(etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> &bin,
               const uint_fast8_t dataLength, const uint_fast8_t handlingCode) {
    setContents(bin, dataLength, handlingCode);
}

/* Getters */
#pragma region Getters

etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> Packet::getRawPacket() {
    /*assert(this->lengthValid >= 0 &&
           this->lengthValid <= WTbNetConfig::MAX_PACKET_LENGTH);*/
    return this->dataBuffer;
}

uint_fast8_t Packet::getVersionID() const {
    // Return the version ID after removing the handling code from the header
    return this->dataBuffer[0] >> HANDLE_ID_LEN_BITS;
}

uint_fast8_t Packet::getHandlingCode() const {
    // Return the handling code after removing the version ID from the header
    return this->dataBuffer[0] & HANDLE_ID_NET_MASK;
}

uint_fast8_t Packet::getLengthValid() const { return this->lengthValid; }

#pragma endregion // Getters

/* Setters */
#pragma region Setters

void Packet::setContentsRaw(
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &bin,
    const uint_fast8_t dataLength) {
    std::copy(bin.begin() + 1, bin.end(), this->dataBuffer.begin());
    this->lengthValid = dataLength;
}

void Packet::setContents(
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> &bin,
    const uint_fast8_t dataLength, const uint_fast8_t handlingCode) {
    // Validate handling Code
    assert(handlingCode <= 0b00011111);
    /*if (handlingCode <= HANDLE_ID_MAX_VAL) {
        handlingCode = handlingCode && HANDLE_ID_NET_MASK;
    }*/
    // Set the header
    dataBuffer[0] = (HEADER_VERSION << HANDLE_ID_LEN_BITS) + handlingCode;

    // Validate the data length
    if (dataLength > WTbNetConfig::MAX_PACKET_DATA_LENGTH) {
        ESP_LOGE(TAG, "Bad Packet Length: %d", dataLength);
        return;
    }

    // Copy the data
    std::copy(this->dataBuffer.begin() + 1, this->dataBuffer.end(),
              bin.begin());
    this->lengthValid = dataLength;
}

#pragma endregion // Setters
