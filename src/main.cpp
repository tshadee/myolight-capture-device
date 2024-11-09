#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include "ADS8686S_SPI_Handler.h"
#include "Battery.h"
#include "COMMON_DEFS.h"
#include "LM27762_Handler.h"

const char* ssid = "ESP32C6T-WIFI";
const char* password = "qw12er34";

const unsigned long intervalMicros = (1000000.0 / WIFI_TRANSACTION_FREQ);
unsigned long previousMicros = 0;
unsigned long counter = 0;

uint32_t wifiBuffer[WIFI_BUFFER_SIZE_BYTES];  // BUFFER_SIZE*32 bits per transmission
uint16_t bufferIndex = 0;
WiFiServer server(WIFI_PORT);  // port DOOM 666

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

    // Power supply 1 initialisation
    LM27762 AFE_PWR(D3);
    AFE_PWR.enableIC();  // enable +-2V5 power supply for analogue front end

    // Power supply 2 initialisation

    // SPI initialisation
    SPIClass vspi(FSPI);
    vspi.begin(D8, D9, D10, D7);
    vspi.beginTransaction(SPISettings(
        SPI_FREQ, MSBFIRST, SPI_MODE2)); /* CPOL = 1, CPHA = 0. TODO: SPI Frequency can be
                                       increased to 50 MHz depending on load capacitance */

    // ADC initialisation
    ADS8686S_SPI_Handler ADC(D7, D6, D5, D4, &vspi);
    delayMicroseconds(1500);  // wait for power supply to stabilise and pulse RESET
    try
    {
        ADC.configureADC();
    }
    catch (...)
    {
        log_v("ADS8686S Configuration Failed");  // error verbose
    };

    // Wi-Fi initialisation
    if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, WIFI_SSID_HIDDEN, WIFI_CONNECTIONS,
                     false, WIFI_AUTH_WPA2_PSK))  // start access point
    {
        log_v("Soft AP creation failed.");  // error verbose
    };
    IPAddress myIP = WiFi.softAPIP();  // set IP
    Serial.print("AP IP Address:");
    Serial.println(myIP);
    server.begin();  // start TCP server

    Serial.println("softAP config done.");
};

void loop()
{
    WiFiClient client = server.accept();  // listen for incoming clients
    if (client)
    {
        Serial.println("New Client.");  // print a message out the serial port
        while (client.connected())
        {  // loop while the client's connected
            unsigned long currentMicros = micros();
            if (currentMicros - previousMicros >= intervalMicros)
            {
                previousMicros = currentMicros;
                for (bufferIndex = 0; bufferIndex < WIFI_BUFFER_SIZE_BYTES; bufferIndex++)
                {
                    counter++;
                    wifiBuffer[bufferIndex] = counter % 65536;
                }
                client.write((uint8_t*)wifiBuffer, sizeof(wifiBuffer));
            }
        }
        // close the connection:
        client.stop();
        Serial.println("\nClient Disconnected.");
    }
};
