#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include "ADS8686S_SPI_Handler.h"
#include "COMMON_DEFS.h"
#include "driver/gpio.h"  // GPIO drive strength functions

#define CONVST D4
#define BUSY D5  // g23
#define RST D6   // g16

#define MOSI D10
#define MISO D9
#define SCLK D8
#define CS D7

const unsigned long intervalMicros = (1000000 / WIFI_TRANSACTION_FREQ);
unsigned long previousMicros = 0;
unsigned long counter = 0;
unsigned long numPacket = 0;
WiFiServer server(WIFI_PORT);  // port DOOM 666

void pinSetup();

SPIClass* vspi = NULL;
ADS8686S_SPI_Handler* ADC = NULL;

void setup()
{
    pinSetup();
    esp_log_level_set("*", ESP_LOG_INFO);
    delay(5000);
    while (!Serial.available());  // wait for the computer
    Serial.begin(9600);

    // FSPI Initialisation
    try
    {
        vspi = new SPIClass(FSPI);
        vspi->begin(SCLK, MISO, MOSI, -1);
    }
    catch (...)
    {
        log_e("FSPI Bus Assignment Critical Failure");
    };

    // ADC Initialisation
    try
    {
        ADC = new ADS8686S_SPI_Handler(GPIO_NUM_17, GPIO_NUM_16, GPIO_NUM_23, GPIO_NUM_22, vspi);
        ADC->configureADC();
    }
    catch (...)
    {
        log_e("ADC Configuration Critical Failure");
    };

    // Wi-Fi initialisation
    if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, WIFI_SSID_HIDDEN, WIFI_CONNECTIONS,
                     false, WIFI_AUTH_WPA2_PSK))  // start access point
    {
        log_e("Soft AP creation failed.");
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
                ADC->initiateSample();
                uint16_t* dataReceived = ADC->getReceiveBuffer();
                client.write((uint8_t*)dataReceived, 8 * sizeof(uint16_t));
                Serial.printf("Packet no: %d \r", numPacket++);
            }
        }
        // close the connection:
        client.stop();
        Serial.println("\nClient Disconnected.");
    }
};

void pinSetup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(CONVST, OUTPUT);
    pinMode(BUSY, INPUT);
    pinMode(RST, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(MISO, INPUT);
    pinMode(SCLK, OUTPUT);
    pinMode(CS, OUTPUT);
    digitalWrite(RST, LOW);
    digitalWrite(CONVST, LOW);
    gpio_set_drive_capability(GPIO_NUM_19,
                              GPIO_DRIVE_CAP_3);  // set SCLK drive to max
    gpio_set_drive_capability(GPIO_NUM_18,
                              GPIO_DRIVE_CAP_3);  // set MOSI drive to max
    gpio_set_drive_capability(GPIO_NUM_17,
                              GPIO_DRIVE_CAP_3);  // set CS drive to max
};
