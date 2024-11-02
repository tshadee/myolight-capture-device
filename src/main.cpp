#include <Arduino.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include "ADS8686S_SPI_Handler.h"
#include "COMMON_DEFS.h"

const char* ssid = "ESP32C6T-WIFI";
const char* password = "qw12er34";

const unsigned long intervalMicros = (1000000.0 / WIFI_TRANSACTION_FREQ);
unsigned long previousMicros = 0;
unsigned long counter = 0;

uint32_t wifiBuffer[WIFI_BUFFER_SIZE_BYTES];  // BUFFER_SIZE*32 bits per transmission
uint16_t bufferIndex = 0;
WiFiServer server(WIFI_PORT);  // port DOOM 666
// MOSI 18
// MISO 20
// SCK 19
// SS 21

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);

    if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, WIFI_SSID_HIDDEN, WIFI_CONNECTIONS,
                     false, WIFI_AUTH_WPA2_PSK))  // start access point
    {
        log_e("Soft AP creation failed.");  // error if unsuccessful
    };
    IPAddress myIP = WiFi.softAPIP();  // set IP
    Serial.print("AP IP Address:");
    Serial.println(myIP);
    server.begin();  // start TCP server

    Serial.println("softAP config done.");
}

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
}
