#include <Arduino.h>
#include <SPI.h>

#include "ADS8686S_SPI_Handler.h"
#include "COMMON_DEFS.h"
#include "driver/gpio.h"  // GPIO drive strength functions

#define CONVST D4
#define BUSY D5
#define RST D6

#define MOSI D10
#define MISO D9
#define SCLK D8
#define CS D7

SPIClass* vspi = NULL;
ADS8686S_SPI_Handler* ADC = NULL;

static const uint16_t SEQ_STACK_2_CHK = SEQ_STACK_1_DEFAULT_CFG & READBACK_MASK;

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
    while(!Serial.available()); //wait for the computer
    Serial.begin(115200);

    try
    {
        vspi = new SPIClass(FSPI);
        vspi->begin(SCLK, MISO, MOSI, -1);

        ADC = new ADS8686S_SPI_Handler(GPIO_NUM_17, RST, BUSY, CONVST, vspi);
        ADC->configureADC();
    }
    catch (...)
    {
        log_e("ADC SPI Configuration Failed");
    };
};

void loop() {

};
