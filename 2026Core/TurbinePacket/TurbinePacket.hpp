#ifndef TURBINE_PACKET_HPP
#define TURBINE_PACKET_HPP

#include <Arduino.h>

// Sender IDs
const uint8_t SENDER_NACELLE = 1;
const uint8_t SENDER_LOADBOX = 2;

// =====================
// NACELLE → LOADBOX
// =====================
struct NacellePacket {
  int16_t rpm;
};

// =====================
// LOADBOX → NACELLE
// =====================
struct LoadboxPacket {
  uint8_t safety;
  // todo add power stuff
};

// =====================
// Utility functions
// =====================

// Build packets
void makeNacellePacket(
 NacellePacket &packet,
 int16_t rpm
 );

void makeLoadboxPacket(
  LoadboxPacket &packet,
  uint8_t safety
  // todo add power stuff
);

// Debug printing
void printNacellePacket(const NacellePacket &packet, Stream &out);
void printLoadboxPacket(const LoadboxPacket &packet, Stream &out);

#endif // TURBINE_PACKET_HPP