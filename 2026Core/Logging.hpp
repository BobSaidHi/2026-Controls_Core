#include <portmacro.h>
#include <projdefs.h>
#include <semphr.h>

namespace Logging {
    inline SemaphoreHandle_t gLogMutex = nullptr;
    inline constexpr uint_fast16_t LOG_MUTEX_WAIT_MS = 20;

    inline bool initLoggingMutex() {
        if (gLogMutex == nullptr) {
            gLogMutex = xSemaphoreCreateMutex();
        }
        return gLogMutex != nullptr;
    }

    inline bool
    takeLogMutex(TickType_t waitTicks = pdMS_TO_TICKS(LOG_MUTEX_WAIT_MS)) {
        return (gLogMutex != nullptr) &&
               (xSemaphoreTake(gLogMutex, waitTicks) == pdTRUE);
    }

    inline void giveLogMutex() {
        if (gLogMutex != nullptr) {
            xSemaphoreGive(gLogMutex);
        }
    }
} // namespace Logging

#define LOGV_LOCKED(tag, fmt, ...)                                             \
    do {                                                                       \
        if (Logging::takeLogMutex()) {                                         \
            ESP_LOGV(tag, fmt, ##__VA_ARGS__);                                 \
            Logging::giveLogMutex();                                           \
        }                                                                      \
    } while (0)

#define LOGD_LOCKED(tag, fmt, ...)                                             \
    do {                                                                       \
        if (Logging::takeLogMutex()) {                                         \
            ESP_LOGD(tag, fmt, ##__VA_ARGS__);                                 \
            Logging::giveLogMutex();                                           \
        }                                                                      \
    } while (0)

#define LOGI_LOCKED(tag, fmt, ...)                                             \
    do {                                                                       \
        if (Logging::takeLogMutex()) {                                         \
            ESP_LOGI(tag, fmt, ##__VA_ARGS__);                                 \
            Logging::giveLogMutex();                                           \
        }                                                                      \
    } while (0)

#define LOGW_LOCKED(tag, fmt, ...)                                             \
    do {                                                                       \
        if (Logging::takeLogMutex()) {                                         \
            ESP_LOGW(tag, fmt, ##__VA_ARGS__);                                 \
            Logging::giveLogMutex();                                           \
        }                                                                      \
    } while (0)

#define LOGE_LOCKED(tag, fmt, ...)                                             \
    do {                                                                       \
        if (Logging::takeLogMutex()) {                                         \
            ESP_LOGE(tag, fmt, ##__VA_ARGS__);                                 \
            Logging::giveLogMutex();                                           \
        }                                                                      \
    } while (0)
