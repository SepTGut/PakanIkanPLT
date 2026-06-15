#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32
    void initWebPortal();
    void handleWebRequests();
#endif

#endif
