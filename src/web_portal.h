/**
 * @file web_portal.h
 * @brief WiFi configuration web portal interface (ESP32 only)
 *
 * Provides a captive portal for configuring WiFi credentials on first boot.
 * The ESP32 starts as an AP ("PakanIkan-Config") with a web form.
 * After saving credentials, it restarts and connects to the specified network.
 *
 * This module is only available on ESP32 platforms. On Arduino Uno,
 * the functions are stubbed out.
 */

#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <Arduino.h>

#ifdef ARDUINO_ARCH_ESP32
    /**
     * @brief Start the WiFi configuration portal.
     *        Creates an AP with captive DNS and serves a configuration web page.
     */
    void initWebPortal();

    /**
     * @brief Handle incoming web/DNS requests. Call every loop() iteration.
     */
    void handleWebRequests();
#endif

#endif // WEB_PORTAL_H
