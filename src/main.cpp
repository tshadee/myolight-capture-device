#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include "ADS8686S_SPI_Handler.h"
#include "COMMON_DEFS.h"
#include "driver/gpio.h"  // GPIO drive strength functions
#include "driver/timer.h"

#define MXS1 D0  // g0
#define MXEN D1  // g1
#define MXS2 D2  // g2

#define POWEREN D3  // g21
#define BUSY D4     // g22
#define CONVST D5   // g23
#define RST D6      // g16

#define MOSI D10  // g18
#define MISO D9   // g20
#define SCLK D8   // g19
#define CS D7     // g17

// Channel Map: 1 , 3 , 5 , 7 , 0 , 2 , 4 , 6

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

struct Config
{
    int sample_rate;
    int adc_range;
    int operation_mode;
};

// Global Variables
volatile bool sampleReady = false;  // Flag set by the ISR
WiFiServer server(WIFI_PORT);       // port DOOM
// WiFiServer debug(WIFI_DEBUG);       // port DOOM-1
// WiFiClient serverClient;            // all data go to this
// WiFiClient debugClient;             // everything else
SPIClass* vspi = NULL;
ADS8686S_SPI_Handler* ADC = NULL;
SysState currentState = IDLE;
SysState previousState = IDLE;
hw_timer_t* timer = NULL;  // Timer handler
unsigned long numPacket = 0;
int timer_interval_us =
    1000000 / 500;  // sampling and wifi will default to 500 hz unless changed by config update
bool singleSampleFlag = false;
int MUX_CH = 1;
int SampleColumn = 1;

void pinSetup();
void configUpdater(String configData, ADS8686S_SPI_Handler* ADC);
void setupTimer();
void stopTimer();
void defaultOperation();
void singleColumn(int col);

void IRAM_ATTR onTimer() { sampleReady = true; };

void setup()
{
    pinSetup();
    esp_log_level_set("*", ESP_LOG_INFO);
    delay(5000);
    if (Serial.available())
    {
        Serial.begin(115200);
    }

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
    // debug.begin();
    log_i("SAP CFG GOOD");
    Serial.print("AP IP ADDR: ");
    Serial.println(myIP);
};

void loop()
{
    WiFiClient client = server.accept();  // listen for incoming clients
    if (client)
    {
        // client.write("Connected");
        log_i("Client Connected");
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
                    // client.write("START");
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
                    // client.write("STOP");
                    currentState = IDLE;
                    stopTimer();
                }
                else if (command == "CONFIG")
                {
                    log_i("CONFIG GOT");
                    // client.write("CONFIG");
                    currentState = CONFIGURING;
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
                        singleSampleFlag ? ADC->initiateSingleSample(MUX_CH)
                                         : ADC->initiate4Sample();
                        ADC->setReceiveBuffer(32, numPacket);
                        const uint16_t* dataReceived = ADC->getReceiveBuffer();
                        client.write((uint8_t*)dataReceived, 33 * sizeof(uint16_t));
                        numPacket++;
                        if (numPacket % 50 == 0)
                        {
                            Serial.println(numPacket);
                        };
                    }
                    break;
                case CONFIGURING:
                    unsigned long startTime = millis();
                    String configData = "";
                    while ((millis() - startTime) < 2000)
                    {
                        if (client.available())
                        {
                            configData = client.readStringUntil('\n');
                            configData.trim();
                            if (!configData.isEmpty())
                            {
                                log_i("Received Config Data");
                                configUpdater(configData, ADC);
                                log_i("Successfully updated ADC with new config data");
                                // client.write("Config Done");
                                currentState = IDLE;
                            }
                            break;
                        }
                    }
                    if (configData.isEmpty())
                    {
                        log_w("No config received within timeout interval");
                        currentState = IDLE;
                    }
                    break;
            }
            previousState = currentState;
        }
        // close the connection:
        client.stop();
        log_i("Client Disconnected");
        numPacket = 0;
    }
};

void setupTimer()
{
    // Initialize the hardware timer
    timer = timerBegin(TIMER_SCALE);  // timer 0, count up
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, timer_interval_us, true, 0);  // auto-reload true
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

void configUpdater(String configData, ADS8686S_SPI_Handler* ADC)
{
    int configArr[3] = {0};
    int index = 0;
    char configBuffer[configData.length() + 1];
    configData.toCharArray(configBuffer, sizeof(configBuffer));

    char* token = strtok(configBuffer, ",");
    while (token != NULL && index < 3)
    {
        configArr[index] = atoi(token);
        index++;
        token = strtok(NULL, ",");
    };
    Config cfg = {configArr[0], configArr[1], configArr[2]};

    const int SAMPLE_RATE_MAP[] = {5000, 2000, 1000, 500};  // 200,500,1000,2000 Hz
    timer_interval_us =
        (cfg.sample_rate >= 0 && cfg.sample_rate < 4) ? SAMPLE_RATE_MAP[cfg.sample_rate] : 5000;

    std::unordered_map<int, std::array<uint16_t, 4>> ADC_RANGE_MAP = {
        {0, {0x8855U, 0x8A55U, 0x8C55U, 0x8E55U}},  // 2V
        {1, {0x88AAU, 0x8AAAU, 0x8CAAU, 0x8EAAU}},  // 4V
        {2, {0x88FFU, 0x8AFFU, 0x8CFFU, 0x8EFFU}}   // 8V
    };
    if (ADC_RANGE_MAP.find(cfg.adc_range) != ADC_RANGE_MAP.end())
    {
        auto& config_vals = ADC_RANGE_MAP[cfg.adc_range];
        for (size_t i = 0; i < config_vals.size(); i++)
        {
            ADC->setConfigElem(config_vals[i], i + 1);
        }
    };

    auto singleColumn = [&](int col)
    {
        ADC->setConfigElem(0x846CU, 0);  // OSR set to 011b (8 samples OSR)
        MUX_CH = col;
        singleSampleFlag = true;
        log_i("Single column set");
    };

    // turn on OSR and lock MUX to one channel
    std::unordered_map<int, std::function<void()>> OPERATION_MODE_MAP = {
        {0, []() { defaultOperation(); }}, {1, [&]() { singleColumn(1); }},
        {2, [&]() { singleColumn(2); }},   {3, [&]() { singleColumn(3); }},
        {4, [&]() { singleColumn(4); }},
    };

    if (OPERATION_MODE_MAP.find(cfg.operation_mode) != OPERATION_MODE_MAP.end())
    {
        OPERATION_MODE_MAP[cfg.operation_mode]();
    };

    try
    {
        ADC->configureADC();
    }
    catch (...)
    {
        log_e("Updated Configuration Failure");
    }
};

void defaultOperation()
{
    singleSampleFlag = false;
    log_i("Set to default operation");
};

void pinSetup()
{
    pinMode(MXS1, OUTPUT);
    pinMode(MXEN, OUTPUT);
    pinMode(MXS2, OUTPUT);
    pinMode(POWEREN, OUTPUT);
    pinMode(BUSY, INPUT);
    pinMode(CONVST, OUTPUT);
    pinMode(RST, OUTPUT);

    pinMode(MOSI, OUTPUT);
    pinMode(MISO, INPUT);
    pinMode(SCLK, OUTPUT);
    pinMode(CS, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(POWEREN, HIGH);
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
