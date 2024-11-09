#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include "ADS8686S_SPI_Handler.h"
#include "Battery.h"
#include "COMMON_DEFS.h"
#include "PowerSupply_Handler.h"

const char* ssid = "ESP32C6T-WIFI";
const char* password = "qw12er34";

const unsigned long intervalMicros = (1000000 / WIFI_TRANSACTION_FREQ);
unsigned long previousMicros = 0;
unsigned long counter = 0;

uint32_t wifiBuffer[WIFI_BUFFER_SIZE_BYTES];  // BUFFER_SIZE*32 bits per transmission
uint16_t bufferIndex = 0;
WiFiServer server(WIFI_PORT);  // port DOOM 666

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    esp_log_level_set("*", ESP_LOG_INFO);

    delay(6000);
    if (Serial.available())
    {
        Serial.begin(115200);
    };

    // Battery!
    Battery lipo(GPIO_NUM_4);
    float vbattf = lipo.readBatt();
    if (vbattf < 3.7)
    {
        log_i("VBATT = %.3f", vbattf);  // This will NEVER be true. But if it is, :shrugg:
    };

    // Power supply init
    PowerSupply PWR(D3);
    PWR.enableIC();  // enable all power supplies (LM27762 and TPS61240)
    log_i("Power ICs started");

    // SPI initialisation
    SPIClass hspi(HSPI);
    hspi.begin(D8, D9, D10, D7);
    hspi.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE2));
    log_i("HSPI Bus Started");
    /* CPOL = 1, CPHA = 0.
     * TODO: SPI Frequency can be increased to 50 MHz depending on load capacitance.
     * FIXME: This holds the SPI bus and does not release it. In our case it doesn't
     * matter as only one chip uses the SPI bus but if we had multi-SPI consider releasing the bus.
     */

    // ADC initialisation
    ADS8686S_SPI_Handler ADC(D7, D6, D5, D4, &hspi);
    try
    {
        ADC.configureADC();
    }
    catch (...)
    {
        log_e("ADS8686S Configuration Failed");  // error verbose
    };

    // hspi.endTransaction();

    // Wi-Fi initialisation
    if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, WIFI_SSID_HIDDEN, WIFI_CONNECTIONS,
                     false, WIFI_AUTH_WPA2_PSK))  // start access point
    {
        log_e("Soft AP creation failed.");  // error verbose
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
