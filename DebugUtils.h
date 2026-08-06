#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

// Debug switch: 1=enabled, 0=disabled.
//
// Important for this project:
//   - FOUR_G_SERIAL is Serial0 and is connected to the 4G module.
//   - The 4G module uses transparent MQTT mode, so anything written to
//     Serial0 can be uploaded to the server.
//   - On ESP32-C3, Serial is Serial0 unless "USB CDC On Boot" is enabled.
//
// Therefore debug output is enabled only when Serial is USB CDC. If USB CDC is
// off, the debug macros become no-ops instead of leaking logs to MQTT.
#define DEBUG_MODE 1

#define DEBUG_PORT Serial

#if DEBUG_MODE && defined(ARDUINO_USB_CDC_ON_BOOT) && ARDUINO_USB_CDC_ON_BOOT
  #define debugBegin(baud) DEBUG_PORT.begin(baud)
  #define debugPrint(...) DEBUG_PORT.print(__VA_ARGS__)
  #define debugPrintln(...) DEBUG_PORT.println(__VA_ARGS__)
  #define debugPrintf(...) DEBUG_PORT.printf(__VA_ARGS__)
#else
  #define debugBegin(baud)
  #define debugPrint(...)
  #define debugPrintln(...)
  #define debugPrintf(...)
#endif

#endif
