#ifndef TURBINE_PACKET_HPP
#define TURBINE_PACKET_HPP

#include <Arduino.h>

#include <utility>

// Sender IDs
const uint8_t SENDER_NACELLE = 1;
const uint8_t SENDER_LOADBOX = 2;

// Estop types
enum class ESTOP_TYPE_NET : uint8_t {
    NONE = 0,
    BUTTON = 1,
    LOAD_DISCONNECT_I = 2,
    LOAD_DISCONNECT_E = 3,
    RESERVED = static_cast<uint8_t>(-1)
};

enum class ESTOP_TYPE_FAST : uint_fast8_t {
    NONE = static_cast<uint_fast8_t>(std::to_underlying(ESTOP_TYPE_NET::NONE)),
    BUTTON =
        static_cast<uint_fast8_t>(std::to_underlying(ESTOP_TYPE_NET::BUTTON)),
    LOAD_DISCONNECT_I = static_cast<uint_fast8_t>(
        std::to_underlying(ESTOP_TYPE_NET::LOAD_DISCONNECT_I)),
    LOAD_DISCONNECT_E = static_cast<uint_fast8_t>(
        std::to_underlying(ESTOP_TYPE_NET::LOAD_DISCONNECT_E)),
    RESERVED =
        static_cast<uint_fast8_t>(std::to_underlying(ESTOP_TYPE_NET::RESERVED))
};

// =====================
// NACELLE → LOADBOX
// =====================
struct NacellePacket {
    int16_t rpm;
    int16_t angularAccel_RPMPS;
};

// =====================
// LOADBOX → NACELLE
// =====================
// DONE: add power stuff
struct LoadboxPacket {
    int16_t d_mVPS;
    int16_t current_mA;
    int16_t dIPS;
    ESTOP_TYPE_NET safety;
};

// =====================
// Utility functions
// =====================

// Build packets
void makeNacellePacket(NacellePacket &packet, int16_t rpm,
                       int16_t angularAccel_RPMPS);

// DONE: add power stuff
void makeLoadboxPacket(LoadboxPacket &packet, int16_t d_mVPS,
                       int16_t current_mA, int16_t dIPS, ESTOP_TYPE_NET safety);

// Debug printing
void printNacellePacket(const NacellePacket &packet, Stream &out);
void printLoadboxPacket(const LoadboxPacket &packet, Stream &out);

#endif // TURBINE_PACKET_HPP