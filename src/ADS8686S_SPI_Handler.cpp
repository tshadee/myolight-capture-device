
#include "ADS8686S_SPI_Handler.h"

// #include <driver/spi_master.h>

ADS8686S_SPI_Handler::ADS8686S_SPI_Handler(uint8_t CS, uint8_t RST, uint8_t BUSY, uint8_t CONVST,
                                           SPIClass* spiInstance)
    : CS(CS),
      RST(RST),
      CONVST(CONVST),
      BUSY(BUSY),
      vspi(spiInstance),
      configArr{CONFIG_DEFAULT_CFG,      RANGE_A1_DEFAULT_CFG,    RANGE_A2_DEFAULT_CFG,
                RANGE_B1_DEFAULT_CFG,    RANGE_B2_DEFAULT_CFG,    LPF_DEFAULT_CFG,
                SEQ_STACK_0_DEFAULT_CFG, SEQ_STACK_1_DEFAULT_CFG, SEQ_STACK_2_DEFAULT_CFG,
                SEQ_STACK_3_DEFAULT_CFG}

{
    std::fill(std::begin(rxBuffer), std::end(rxBuffer), 0);
    pinMode(CS, OUTPUT);
    pinMode(RST, OUTPUT);
    pinMode(CONVST, OUTPUT);
    pinMode(BUSY, INPUT);
    digitalWrite(CS, HIGH);
    digitalWrite(RST, LOW);
    digitalWrite(CONVST, LOW);
    delay(10);
    configureSPI();
};

void ADS8686S_SPI_Handler::configureSPI(void) {

};

uint16_t ADS8686S_SPI_Handler::getConfigArr(int element) { return configArr[element]; };
uint16_t ADS8686S_SPI_Handler::getReceiveBuffer(int element) { return rxBuffer[element]; };
uint16_t* ADS8686S_SPI_Handler::getConfigArr(void) { return configArr; };
uint16_t* ADS8686S_SPI_Handler::getReceiveBuffer(void) { return rxBuffer; };

void ADS8686S_SPI_Handler::writeRegister(uint8_t REGADDR, uint8_t DATA)
{
    if (!digitalRead(BUSY))
    {
        uint16_t transferData = (0x8000 | (REGADDR << 9)) | DATA;
        digitalWrite(CS, LOW);
        vspi->transfer16(transferData);
        digitalWrite(CS, HIGH);
    };
};

uint16_t ADS8686S_SPI_Handler::readRegister(uint8_t REGADDR)
{
    if (!digitalRead(BUSY))
    {
        uint16_t transferData = (REGADDR << 1) | 0x00;
        digitalWrite(CS, LOW);
        vspi->transfer16(transferData);
        uint16_t dataIn = vspi->transfer16(0x0000);
        digitalWrite(CS, HIGH);
        return dataIn;
    }
    else
    {
        return 0x0000;
    }
};

void ADS8686S_SPI_Handler::setConfigArr(uint16_t* data)
{
    for (int i = 0; i < cfgArrayDepth; i++)
    {
        configArr[i] = data[i];
    };
};

void ADS8686S_SPI_Handler::clearReceiveBuffer(void)
{
    std::fill(std::begin(rxBuffer), std::end(rxBuffer), 0);
};

// Call this function ONCE upon startup. ADC will undergo full reset and load in the
// configration profile. Make sure to set the cfgProfile in setConfigArr() before calling this
// function
void ADS8686S_SPI_Handler::configureADC(void)
{
    delay(500);  // 0.5s delay in case this is called right after power supply enable
    digitalWrite(RST, LOW);
    delayMicroseconds(100);  // FULL RESET. Hold RST low for 50 us
    digitalWrite(RST, HIGH);
    delayMicroseconds(500);  // From FREST to CS falling is 240 us minimum (500 for safety)
    if (!digitalRead(BUSY))
    {
        // writes to register (ADC output invalid)
        for (int i = 0; i < cfgArrayDepth; i++)
        {
            digitalWrite(CS, LOW);
            vspi->transfer16(configArr[i]);
            digitalWrite(CS, HIGH);
            delayMicroseconds(1);
        };

        // register readback for verification (+1 cycle to readback)
        uint16_t transferData = (configArr[0] & READBACK_MASK);
        digitalWrite(CS, LOW);
        vspi->transfer16(transferData);
        digitalWrite(CS, HIGH);

        for (int i = 1; i < cfgArrayDepth; i++)
        {
            uint16_t transferData = (configArr[i] & READBACK_MASK);
            digitalWrite(CS, LOW);
            uint16_t receivedData = vspi->transfer16(transferData);
            digitalWrite(CS, HIGH);
            if (receivedData != configArr[i - 1])
            {
                if (Serial.available())
                {
                    Serial.print("Configuration mismatch at element: ");
                    Serial.println(i - 1);
                    Serial.print("Received Data: ");
                    Serial.println(receivedData, BIN);
                    Serial.print("Expected Data: ");
                    Serial.println(configArr[i - 1], BIN);
                }
                else
                {
                    log_w("Configuration mismatch at element: %d", i - 1);
                    log_w("Received Data : %d", receivedData);
                    log_w("Expected Data : %d", configArr[i - 1]);
                }
                break;
            };
        };

        digitalWrite(CONVST, HIGH);  // pull CONVST high to initiate dummy conversion
        digitalWrite(CONVST, LOW);   // this toggle takes like 1 us total
        while (digitalRead(BUSY));   // this is most likely already off
        vspi->transfer16(0x0000);    // you could do a dummy read but this is to set the SEQEN
                                     // pointer to the top of the stack
        clearReceiveBuffer();
        // ADC should be ready for use after this.
    };
};

void ADS8686S_SPI_Handler::initiateSample(void)
{
    digitalWrite(CONVST, HIGH);
    digitalWrite(CONVST, LOW);
    while (digitalRead(BUSY));  // TODO: BUSY to CS falling minimum 20 ns.
    digitalWrite(CS, LOW);
    for (int i = 0; i < rxBufferDepth; i++)
    {
        rxBuffer[i] = vspi->transfer16(
            0x0000);  // load all 8 samples (7 valid) into rxBuffer. Results from
                      // CHA and CHB are interlaced so check ADS_REG_DEFAULTS carefully
    }
    digitalWrite(CS, HIGH);
};
