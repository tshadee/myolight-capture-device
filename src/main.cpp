#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include "ADS8686S_SPI_Handler.h"
#include "COMMON_DEFS.h"
#include "driver/gpio.h"  // GPIO drive strength functions
#include "driver/timer.h"

#define CONVST D5  // g23
#define BUSY D4    // g22
#define RST D6     // g16

#define MOSI D10  // g18
#define MISO D9   // g20
#define SCLK D8   // g19
#define CS D7     // g17

#define MXS1 D0
#define MXS2 D2
#define MXEN D1

// Timer parameters
#define TIMER_DIVIDER 80  // Divides 80MHz clock to 1MHz (1us tick)
#define TIMER_SCALE (80000000 / TIMER_DIVIDER)
#define TIMER_INTERVAL_US (1000000 / WIFI_TRANSACTION_FREQ)  // in microseconds

enum SysState
{
    IDLE,
    SENDING_DATA,
    CONFIGURING
};

// Global Variables
volatile bool sampleReady = false;  // Flag set by the ISR
WiFiServer server(WIFI_PORT);       // port DOOM 666
SPIClass* vspi = NULL;
ADS8686S_SPI_Handler* ADC = NULL;
SysState currentState = IDLE;
SysState previousState = IDLE;
hw_timer_t* timer = NULL;  // Timer handler
unsigned long numPacket = 0;

void pinSetup();

void IRAM_ATTR onTimer() { sampleReady = true; };

void setupTimer()
{
    // Initialize the hardware timer
    timer = timerBegin(TIMER_SCALE);  // timer 0, count up
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, TIMER_INTERVAL_US, true, 0);  // auto-reload true
}

void stopTimer()
{
    if (timer != NULL)
    {
        timerDetachInterrupt(timer);
        timerEnd(timer);
        timer = NULL;
    }
}

void setup()
{
    pinSetup();
    esp_log_level_set("*", ESP_LOG_INFO);
    delay(10000);
    while (!Serial.available());  // wait for the computer
    Serial.begin(115200);

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
        ADC = new ADS8686S_SPI_Handler(GPIO_NUM_17, GPIO_NUM_16, GPIO_NUM_22, GPIO_NUM_23, vspi,
                                       GPIO_NUM_0, GPIO_NUM_2, GPIO_NUM_1);
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
    server.begin();                    // start TCP server
    log_i("SAP CFG GOOD");
    Serial.print("AP IP ADDR: ");
    Serial.println(myIP);
};

void loop()
{
    WiFiClient client = server.accept();  // listen for incoming clients
    if (client)
    {
        Serial.println("New Client.");  // print a message out the serial port
        while (client.connected())
        {  // loop while the client's connected
            if (client.available())
            {
                String command = client.readStringUntil('\n');
                command.trim();
                // good ol statemachine
                if (command == "START")
                {
                    log_i("START GOT");
                    switch (previousState)
                    {
                        case SENDING_DATA:
                            currentState = SENDING_DATA;
                            break;
                        case IDLE:
                            currentState = SENDING_DATA;
                            setupTimer();
                            break;
                        case CONFIGURING:
                            currentState = IDLE;
                            break;
                    }
                }
                else if (command == "STOP")
                {
                    log_i("STOP GOT");
                    currentState = IDLE;
                    stopTimer();
                }
                else if (command == "CONFIG")
                {
                    log_i("CONFIG GOT");
                    currentState = CONFIGURING;
                }
                else
                {
                    currentState == IDLE;
                };
            };

            switch (currentState)
            {
                case IDLE:
                    numPacket = 0;
                    break;
                case SENDING_DATA:
                    if (sampleReady)  // polling managed by ISR
                    {
                        sampleReady = false;
                        ADC->initiate4Sample();
                        ADC->setReceiveBuffer(32, numPacket);
                        const uint16_t* dataReceived = ADC->getReceiveBuffer();
                        client.write((uint8_t*)dataReceived, 33 * sizeof(uint16_t));
                        numPacket++;
                    }
                    break;
                case CONFIGURING:

                    break;
            }
            previousState = currentState;
        }
        // close the connection:
        client.stop();
        Serial.println("\nClient Disconnected.");
        numPacket = 0;
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
    pinMode(MXS1, OUTPUT);
    pinMode(MXS2, OUTPUT);
    pinMode(MXEN, OUTPUT);
    digitalWrite(RST, LOW);
    digitalWrite(CONVST, LOW);
    digitalWrite(MXS1, LOW);
    digitalWrite(MXS2, LOW);
    digitalWrite(MXEN, LOW);  // MUX off until necessary
    gpio_set_drive_capability(GPIO_NUM_19,
                              GPIO_DRIVE_CAP_3);  // set SCLK drive to max
    gpio_set_drive_capability(GPIO_NUM_18,
                              GPIO_DRIVE_CAP_3);  // set MOSI drive to max
    gpio_set_drive_capability(GPIO_NUM_17,
                              GPIO_DRIVE_CAP_3);  // set CS drive to max
};
