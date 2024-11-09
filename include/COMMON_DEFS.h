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
#define SPI_FREQ 24000000  // 24 MHz for now. Upper limit is 50 MHz

/*

[FL]
D0  -   MUX SEL 2   (sel2)
D1  -   MUX EN      (sel1)
D2  -   MUX SEL 1   (convst)
D3  -   POWER EN    (busy)
D4  -   CONVST      (rst)
D5  -   BUSY        (cs)
D6  -   RST         (x)

[FR]
5V  -   x
GND -   UNIFIED GND
3V3 -   TO ADC DIGITAL
D10 -   MOSI
D9  -   MISO
D8  -   SCLK
D7  -   CS          (x)

[BR]
GPIO7 - x
GPIO6 - x

[BL]
GPIO5 - x
GPIO4 - (A4) Battery Voltage Measurement

[BS]
BATT+
BATT-

*/

/*

log_e (error)   (lowest)
log_w (warning)
log_i (info)
log_d (debug)
log_v (verbose) (highest)

*/

#endif  // COMMON_DEFS_H
