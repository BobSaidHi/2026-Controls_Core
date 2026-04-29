#include "TurbinePacket.hpp"

// =====================
// Create packets
// =====================

void makeNacellePacket(NacellePacket &packet, int16_t rpm) {
  packet.rpm = rpm;
}

void makeLoadboxPacket(
  LoadboxPacket &packet,
  uint8_t state,
  uint8_t estop,
  uint16_t actuatorPos
) {
  // packet.state = state;
  packet.safety = estop;
  // packet.actuatorPos = actuatorPos;
}

// =====================
// Debug printing
// =====================

void printNacellePacket(const NacellePacket &packet, Stream &out) {
  out.println("---- NacellePacket ----");
  out.print("RPM: ");
  out.println(packet.rpm);
  out.println("-----------------------");
}

void printLoadboxPacket(const LoadboxPacket &packet, Stream &out) {
  // out.println("---- LoadboxPacket ----");
  // out.print("State: ");
  // out.println(packet.state);

  out.print("EStop: ");
  out.println(packet.safety);

  // out.print("ActuatorPos: ");
  // out.println(packet.actuatorPos);

  out.println("-----------------------");
}