#pragma once
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

// Wi-Fi stuff
#define WIFI_SSID "ESP32C6T-softAP"
#define WIFI_PASSWORD "qw12er34"
#define WIFI_BUFFER_SIZE_BYTES 56  // Multiply these two
#define WIFI_TRANSACTION_FREQ 500  // to get the data rate
#define WIFI_PORT 666              // DOOM
#define WIFI_CHANNEL 1             // supports ch 1 - 13
#define WIFI_SSID_HIDDEN false     // actually demands int but :shrugg:
#define WIFI_CONNECTIONS 1         // number of connections to softAP

#endif  // COMMON_DEFS_H
