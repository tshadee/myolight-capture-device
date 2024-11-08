#include "ADS8686S_SPI_Handler.h"

ADS8686S_SPI_Handler::ADS8686S_SPI_Handler(uint8_t SDI, uint8_t SDO, uint8_t CS, uint8_t SCLK,
                                           uint8_t RST, uint8_t BUSY, uint8_t CONVST,
                                           SPIClass* spiInstance)
    : SDI(SDI),
      SDO(SDO),
      CS(CS),
      SCLK(SCLK),
      RST(RST),
      CONVST(CONVST),
      BUSY(BUSY),
      vspi(spiInstance),
      configArr{CONFIG_DEFAULT_CFG,      RANGE_A1_DEFAULT_CFG,    RANGE_A2_DEFAULT_CFG,
                RANGE_B1_DEFAULT_CFG,    RANGE_B2_DEFAULT_CFG,    LPF_DEFAULT_CFG,
                SEQ_STACK_0_DEFAULT_CFG, SEQ_STACK_1_DEFAULT_CFG, SEQ_STACK_2_DEFAULT_CFG,
                SEQ_STACK_3_DEFAULT_CFG} {
          // std::fill(std::begin(txBuffer), std::end(txBuffer), 0);
          // std::fill(std::begin(rxBuffer), std::end(rxBuffer), 0);
      };

void ADS8686S_SPI_Handler::SPI_open(void)
{
    vspi->begin(SCLK, SDI, SDO, CS);
    vspi->beginTransaction(SPISettings(
        SPI_FREQ, MSBFIRST, SPI_MODE2));  // CPOL = 1, CPHA = 0. TODO: SPI Frequency can be
                                          // increased to 50 MHz depending on load capacitance
    pinMode(CS, OUTPUT);
    pinMode(RST, OUTPUT);
    pinMode(CONVST, OUTPUT);
    pinMode(BUSY, INPUT);
    digitalWrite(CS, HIGH);
    digitalWrite(RST, HIGH);
    digitalWrite(CONVST, LOW);
};

void ADS8686S_SPI_Handler::SPI_close(void) { vspi->end(); };

uint16_t ADS8686S_SPI_Handler::getConfigArr(int element) { return configArr[element]; };
uint16_t ADS8686S_SPI_Handler::getTransmitBuffer(int element) { return txBuffer[element]; };
uint16_t ADS8686S_SPI_Handler::getReceiveBuffer(int element) { return rxBuffer[element]; };
uint16_t* ADS8686S_SPI_Handler::getConfigArr(void) { return configArr; };
uint16_t* ADS8686S_SPI_Handler::getTransmitBuffer(void) { return txBuffer; };
uint16_t* ADS8686S_SPI_Handler::getReceiveBuffer(void) { return rxBuffer; };

void ADS8686S_SPI_Handler::writeRegister(uint8_t REGADDR, uint8_t DATA)
{
    if (!digitalRead(BUSY))
    {
        uint16_t transferData = ((0x80 | (REGADDR << 1)) << 8) | DATA;
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
    };
};

void ADS8686S_SPI_Handler::setConfigArr(uint16_t* data)
{
    for (int i = 0; i < cfgArrayDepth; i++)
    {
        configArr[i] = data[i];
    };
};

void ADS8686S_SPI_Handler::setTransmitBuffer(uint16_t* data)
{
    for (int i = 0; i < txBufferDepth; i++)
    {
        txBuffer[i] = data[i];
    };
};

void ADS8686S_SPI_Handler::clearReceiveBuffer(void) {
    // std::fill(std::begin(rxBuffer), std::end(rxBuffer), 0);
};

void ADS8686S_SPI_Handler::configureADC(int configProfile)  // ONLY CALL THIS ONCE
{
    if (!digitalRead(BUSY))
    {
        switch (configProfile)
        {
            case (1):
            {
                // writes to register (ADC output invalid)
                for (int i = 0; i < cfgArrayDepth; i++)
                {
                    digitalWrite(CS, LOW);
                    vspi->transfer16(configArr[i]);
                    digitalWrite(CS, HIGH);
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
                        Serial.print("Configuration mismatch at element: ");
                        Serial.println(i - 1);
                        Serial.print("Received Data: ");
                        Serial.println(receivedData, BIN);
                        Serial.print("Expected Data: ");
                        Serial.println(configArr[i - 1], BIN);
                        break;
                    };
                };
                break;
            };

                digitalWrite(CONVST, HIGH);  // pull CONVST high to initiate dummy conversion
                digitalWrite(CONVST, LOW);   // this toggle takes like 1 us total
                while (digitalRead(BUSY));   // this is most likely already off
                vspi->transfer16(0x0000);  // you could do a dummy read but this is to set the SEQEN
                                           // pointer to the top of the stack
        };
    }
};

void ADS8686S_SPI_Handler::configureArr(int configProfile) {};

void ADS8686S_SPI_Handler::initiateSample(void)
{
    digitalWrite(CONVST, HIGH);
    digitalWrite(CONVST, LOW);
    while (digitalRead(BUSY));
    for (int i = 0; i < rxBufferDepth; i++)
    {
        rxBuffer[i] = vspi->transfer16(0x0000);  // load all 8 samples (7 valid) into rxBuffer
    }
};
