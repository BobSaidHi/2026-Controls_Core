#include "TurbinePacket.hpp"

// =====================
// Create packets
// =====================

void makeNacellePacket(NacellePacket &packet, int16_t rpm,
                       int16_t angularAccell_RPMPS) {
    packet.rpm = rpm;
    packet.angularAccell_RPMPS = angularAccell_RPMPS;
}

void makeLoadboxPacket(LoadboxPacket &packet, int16_t d_mVPS,
                       int16_t current_mA, int16_t dIPS,
                       ESTOP_TYPE_NET safety) {
    packet.d_mVPS = d_mVPS;
    packet.current_mA = current_mA;
    packet.dIPS = dIPS;
    packet.safety = safety;
}

// =====================
// Debug printing
// =====================

void printNacellePacket(const NacellePacket &packet, Stream &out) {
    out.println("---- NacellePacket ----");
    out.print("RPM: ");
    out.println(packet.rpm);
    out.print("dRPM/s: ");
    out.println(packet.angularAccell_RPMPS);
    out.println("-----------------------");
}

void printLoadboxPacket(const LoadboxPacket &packet, Stream &out) {
    out.println("---- LoadboxPacket ----");
    out.print("dV/s: ");
    out.println(packet.d_mVPS);
    out.print("Current: ");
    out.println(packet.current_mA);
    out.print("dC/s: ");
    out.println(packet.dIPS);
    out.print("Safety: ");
    out.println(static_cast<uint_fast8_t>(packet.safety));

    out.println("-----------------------");
}