#include <Arduino.h>
#include <SPI.h>

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

SPIClass* vspi = NULL;
ADS8686S_SPI_Handler* ADC = NULL;

void setup()
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

    esp_log_level_set("*", ESP_LOG_INFO);

    delay(5000);
    while (!Serial.available());  // wait for the computer
    Serial.begin(115200);

    try
    {
        vspi = new SPIClass(FSPI);
        vspi->begin(SCLK, MISO, MOSI, -1);

        ADC = new ADS8686S_SPI_Handler(GPIO_NUM_17, GPIO_NUM_16, GPIO_NUM_23, GPIO_NUM_22, vspi);
        ADC->configureADC();
    }
    catch (...)
    {
        log_e("ADC SPI Configuration Failed");
    };
};

void loop()
{
    delay(10);
    ADC->initiateSample();
    uint16_t* dataReceived = ADC->getReceiveBuffer();
    Serial.print("\r");  // Move cursor to the beginning of the line
    for (int i = 0; i < 8; i++)
    {
        float output = ((int16_t)(dataReceived[i])) * 2.500f / 32768.000f;
        Serial.print(output, 4);
        if (i < 7) Serial.print(", ");  // Add commas between values
    }
    Serial.print("      ");  // Add spaces to clear leftover characters
};
