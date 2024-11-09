#pragma once
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

// Wi-Fi Configuration
#define WIFI_SSID "ESP32C6T-softAP"
#define WIFI_PASSWORD "qw12er34"
#define WIFI_BUFFER_SIZE_BYTES 56  // Multiply these two
#define WIFI_TRANSACTION_FREQ 500  // to get the data rate
#define WIFI_PORT 666              // DOOM
#define WIFI_CHANNEL 1             // supports ch 1 - 13
#define WIFI_SSID_HIDDEN false     // actually demands int but :shrugg:
#define WIFI_CONNECTIONS 1         // number of connections to softAP

// SPI
#define SPI_FREQ 20000000  // 20 MHz for now. Upper limit is 50 MHz

/*

[FL]
D0  -   MUX SEL 2
D1  -   MUX EN
D2  -   MUX SEL 1
D3  -   LM27762 EN
D4  -   CONVST
D5  -   BUSY
D6  -   RST

[FR]
5V  -   x
GND -   UNIFIED GND
3V3 -   TO LM27762
D10 -   MOSI
D9  -   MISO
D8  -   SCLK
D7  -   CS

[BR]
GPIO7
GPIO6

[BL]
GPIO5 - 
GPIO4 - (A4) Battery Voltage Measurement

[BS]
BATT+
BATT-

*/

#endif  // COMMON_DEFS_H
