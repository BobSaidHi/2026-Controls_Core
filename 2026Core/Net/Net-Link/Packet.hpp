#pragma once

/* Imports and Config */
#pragma region Includes, & Config

// C Libraries
#include <etl/array.h>

// Custom Imports
#include "../../CommonConfig.hpp"
// #include "./PacketFormatter_I.hpp"

/**
 * Header Format:
 * Index:      | 7 6 5               | 4 3 2 1 0              |
 * Convention: | 5 6 7               | 4 3 2 1 0              |
 * Name:       | Version ID (3 bits) | Handling Code (5 bits) |
 */

// Header version
constexpr uint_fast8_t HEADER_VERSION = 0b100; // Config
// Check if the header version is less than 3 bits long
constexpr uint_fast8_t HEADER_VER_MAX_VAL = 0b00000111;
static_assert(HEADER_VERSION <= HEADER_VER_MAX_VAL,
              "Header version our of range");
constexpr uint_fast8_t VERSION_LEN_BITS = 3;
constexpr uint_fast8_t HEADER_VERSION_NET_MASK = 0b11100000;

// Header Handle ID
constexpr uint_fast8_t HANDLE_ID_LEN_BITS = 5;
constexpr uint_fast8_t HANDLE_ID_MAX_VAL = 0b00011111;
constexpr uint_fast8_t HANDLE_ID_NET_MASK = 0b00011111;
static_assert(HANDLE_ID_MAX_VAL == HANDLE_ID_NET_MASK,
              "Handle ID max value and net mask should match");

// Double Check
static_assert(VERSION_LEN_BITS + HANDLE_ID_LEN_BITS == 8,
              "Header version and handle ID lengths do not add up to one byte");

#pragma endregion // Includes, & Config

/**
 * @brief Packet class
 * A formatted packet contains a reusable data buffer and a length field to
 * describe how much many bytes of the buffer are valid. The reusable data
 * buffer may be read, directly written too, or a written too through a
 * formatter.
 * @author Noah (@BobSaidHi)
 */
class Packet {
  private:
    /** MARK: Private Variables */
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> dataBuffer = {
        HEADER_VERSION << HANDLE_ID_LEN_BITS}; // Data storage buffer
    // uint8_t ABSLengthUsed = 1; // Length of valid data in the data array in
    // bytes, including the header
    uint_fast8_t lengthValid = 0; // Length of valid data in the data array in bytes

  public:
    /**
     * @see
     * https://stackoverflow.com/questions/54258241/warning-iso-c-forbids-converting-a-string-constant-to-char-for-a-static-c
     */
    static constexpr const char *TAG = "P";
    /* MARK: Public Variables */
    // TODO:
    // https://www.learncpp.com/cpp-tutorial/scoped-enumerations-enum-classes/
    enum PacketType : uint_fast8_t {
        // Other
        RESERVED = 0x00,
        SpeedTest,
        NetTest,
        NTPRequest,
        NTPReply,

        // Plain
        PriorityCmd,
        Cmd,
        Log,
        PriorityData,
        Data,
        LowPriorityData,

        // MessagePack
        // MsgPck_PriorityCmd = 0x02,
        // MsgPck_Cmd = 0x03,
        // MsgPck_Log = 0x04,
        // MsgPck_PriorityData = 0x05,
        // MsgPck_Data = 0x06,
        // MsgPck_LowPriorityData = 0x07,

        // ProtoBuf
        // PBuf_PriorityCmd = 0x12,
        // PBuf_Cmd = 0x13,
        // PBuf_Log = 0x14,
        // PBuf_PriorityData = 0x15,
        // PBuf_Data = 0x16,
        // PBuf_LowPriorityData = 0x17,
    };

    /* MARK: Standard Functions */
    Packet();

    /**
     * @brief Sets the contents of the packet
     * @param bin the data to set
     * @param dataLength the length of the data in bytes
     */
    Packet(etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &bin,
           const uint_fast8_t dataLength);

    /**
     * @brief Sets the contents of the packet
     * @param bin the data to set
     * @param dataLength the length of the data in bytes
     * @param handlingCode the handling code of the packet
     */
    Packet(etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> &bin,
           const uint_fast8_t dataLength, uint_fast8_t handlingCode);

    ~Packet() = default;

    /* MARK: Getters */

    /**
     * @brief Get the data array
     * @returns a byte array of length MAX_ABS_PACKET_LENGTH
     */
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> getRawPacket() const;

    /**
     * @brief Get the length of the data object
     * @returns the length of the valid data in the data array in bytes
     */
    uint_fast8_t getLengthValid() const;

    /**
     * @brief Get the version ID of the packet
     * @returns the version ID of the packet
     */
    uint_fast8_t getVersionID() const;

    /**
     * @brief Get the handling code of the packet
     * @returns the handling code of the packet
     */
    uint_fast8_t getHandlingCode() const;

    /* MARK: Setters */

    /**
     * @brief Sets the contents of the packet
     * @param bin the data to set
     * @param dataLength the length of the data in bytes
     * @deprecated Just use setContents()
     */
    void
    setContentsRaw(etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &bin,
                   const uint_fast8_t dataLength);

    /**
     * @brief Sets the contents of the packet
     * @param bin the data to set
     * @param dataLength the length of the data in bytes
     * @param handlingCode the handling code of the packet
     */
    void
    setContents(etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> &bin,
                const uint_fast8_t dataLength, const uint_fast8_t handlingCode);

    /**
     * @brief Set the data array
     * @param data the data to set
     * @param formatter the PacketFormatter to use
     * @deprecated Prefer manual composition by via setContents(format(data))
     */
    // template <typename Datatype>
    // void setContentsFormatted(const PacketFormatter_I formatter,
    //                           const Datatype data); // todo

}; // class Packet
