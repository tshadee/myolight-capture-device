#pragma once
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

// Wi-Fi Configuration
#define WIFI_SSID "ESP32C6T-softAP"
#define WIFI_PASSWORD "qw12er34"
#define WIFI_BUFFER_SIZE_BYTES 66  // Multiply these two
#define WIFI_TRANSACTION_FREQ 500  // to get the data rate
#define WIFI_PORT 666              // DOOM
#define WIFI_CHANNEL 1             // supports ch 1 - 13
#define WIFI_SSID_HIDDEN false     // actually demands int but :shrugg:
#define WIFI_CONNECTIONS 1         // number of connections to softAP

// SPI
#define SPI_FREQ 2400000  // 2.4 MHz for now. Upper limit is 50 MHz

/*

[FL]
D0  -   MUX SEL 1   (sel2)
D1  -   MUX EN      (sel1)
D2  -   MUX SEL 2   (convst)
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

#define CONFIG_DEFAULT_CFG 0x8460U  // BURSTEN = 1, SEQEN = 1, OSR = xx0b, STATUSEN = 0, CRCEN = 0
#define RANGE_A1_DEFAULT_CFG 0x8855U
#define RANGE_A1_DEFAULT_CFG_BIN 0b1000100001010101
#define RANGE_A2_DEFAULT_CFG 0x8A55U
#define RANGE_B1_DEFAULT_CFG 0x8C55U
#define RANGE_B2_DEFAULT_CFG 0x8E55U
#define LPF_DEFAULT_CFG 0x9A02U
#define SEQ_STACK_0_DEFAULT_CFG 0xC047U
#define SEQ_STACK_1_DEFAULT_CFG 0xC256U
#define SEQ_STACK_2_DEFAULT_CFG 0xC465U
#define SEQ_STACK_3_DEFAULT_CFG 0xC77BU

#define CONFIGURATION_REG_ADDR 0x2U
#define CHANNEL_SEL_REG_ADDR 0x3U
#define RANGE_A1_REG_ADDR 0x4U
#define RANGE_A2_REG_ADDR 0x5U
#define RANGE_B1_REG_ADDR 0x6U
#define RANGE_B2_REG_ADDR 0x7U
#define STATUS_REG_ADDR 0x8U
#define OVERRANGE__A_REG_ADDR 0xAU
#define OVERRANGE__B_REG_ADDR 0xBU
#define LPF_CONFIG_REG_ADDR 0xDU
#define SEQ_STACK_BASE_REG_ADDR 0x20U
#define SEQ_STACK_REG_ADDR(n) static_cast<uint8_t>(SEQ_STACK_BASE_REG + (n))  // until n = 31

#define READBACK_MASK 0b0111111000000000          // 0b011111110_0x00
#define READBACK_MASK_NO_ADDR 0b0000000011111111  // 0x00FF

#define DEVICE_ID_REG 0b0010000000000000

enum cfgProfile
{
    Default,
    SingleSample,
    SingleChannel,
    SweepManualCS,
    SweepBurstSEQEN,
    HighOSR
};

#endif  // COMMON_DEFS_H
