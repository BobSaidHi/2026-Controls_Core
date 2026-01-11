#include <cstdint>
#include <etl/string.h>

namespace Tags {
    /**
     * @brief Enum holding tags for different cod modules to be used a concise
     * logging representation.  Uses the upper 4 bits of a byte.
     */
    enum Tags : uint_fast8_t {
        RESERVED = 0x00,
        NET_APP_NTP = 0x10,
        NET_LINK_ESPNOW = 0x20,
        NET_LINK_UCHI = 0x20,
        NET_LINK_PACKET = 0x20,
        NET_PHY_WLAN = 0x30,
        NET_ADAPTER = 0x40,
        COMMON_CONFIG = 0x50,
        TAGS = 0x60,
        NACELLE_MAIN = 0x70,
        LOAD_MAIN = 0x80,
    };

    /**
     * @brief Decodes the given tag into a human-readable string.
     * @returns A string representing the human-readable form of the tag.
     * TODO: Should this be static?
     */
    static etl::string<20> decode(Tags tag) {
        if (tag == Tags::RESERVED) {
            return "RESERVED";
        } else if (tag == Tags::NET_APP_NTP) {
            return "NET_APP_NTP";
        } else if (tag == Tags::NET_LINK_ESPNOW) {
            return "NET_LINK_ESPNOW";
        } else if (tag == Tags::NET_LINK_UCHI) {
            return "NET_LINK_UCHI";
        } else if (tag == Tags::NET_LINK_PACKET) {
            return "NET_LINK_PACKET";
        } else if (tag == Tags::NET_PHY_WLAN) {
            return "NET_PHY_WLAN";
        } else if (tag == Tags::NET_ADAPTER) {
            return "NET_ADAPTER";
        } else if (tag == Tags::COMMON_CONFIG) {
            return "COMMON_CONFIG";
        } else if (tag == Tags::TAGS) {
            return "TAGS";
        } else if (tag == Tags::NACELLE_MAIN) {
            return "NACELLE_MAIN";
        } else if (tag == Tags::LOAD_MAIN) {
            return "LOAD_MAIN";
        } else {
            return "UNKNOWN";
        }
    }
}; // namespace Tags
