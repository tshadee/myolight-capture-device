#include "ADS8686S_SPI_Handler.h"

ADS8686S_SPI_Handler::ADS8686S_SPI_Handler(int SDI, int SDO, int CS, int SCLK, int RST, int CONVST,
                                           int BUSY, SPIClass& spiInstance)
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
                SEQ_STACK_3_DEFAULT_CFG}
{
    std::fill(std::begin(txBuffer), std::end(txBuffer), 0);
    std::fill(std::begin(rxBuffer), std::end(rxBuffer), 0);
};

void ADS8686S_SPI_Handler::SPI_start(void)
{
    vspi.begin(SCLK, SDI, SDO, CS);
    vspi.setFrequency(SPI_FREQ);
    vspi.setDataMode(SPI_MODE0);
    pinMode(CS, OUTPUT);
    pinMode(RST, OUTPUT);
    pinMode(CONVST, OUTPUT);
    pinMode(BUSY, INPUT);
    digitalWrite(CS, HIGH);
    digitalWrite(RST, HIGH);
    digitalWrite(CONVST, LOW);
};

void ADS8686S_SPI_Handler::SPI_close(void) { vspi.end(); };
uint16_t ADS8686S_SPI_Handler::getConfigArr(int element) { return configArr[element]; };
uint16_t ADS8686S_SPI_Handler::getTransmitBuffer(int element) { return txBuffer[element]; };
uint16_t ADS8686S_SPI_Handler::getReceiveBuffer(int element) { return rxBuffer[element]; };
uint16_t* ADS8686S_SPI_Handler::getConfigArr(void) { return configArr; };
uint16_t* ADS8686S_SPI_Handler::getTransmitBuffer(void) { return txBuffer; };
uint16_t* ADS8686S_SPI_Handler::getReceiveBuffer(void) { return rxBuffer; };

void ADS8686S_SPI_Handler::configureADC(cfgProfile configProfile)
{
    if (!digitalRead(BUSY))
    {
        switch (configProfile)
        {
            case (cfgProfile::Default):
            {
                // writes to register (ADC output invalid)
                for (int i = 0; i < cfgArrayDepth; i++)
                {
                    digitalWrite(CS, LOW);
                    vspi.transfer16(configArr[i]);
                    digitalWrite(CS, HIGH);
                };

                // register readback for verification (+1 cycle to readback)
                uint16_t transferData = (configArr[0] & READBACK_MASK);
                digitalWrite(CS, LOW);
                vspi.transfer16(transferData);
                digitalWrite(CS, HIGH);

                for (int i = 1; i < cfgArrayDepth; i++)
                {
                    uint16_t transferData = (configArr[i] & READBACK_MASK);
                    digitalWrite(CS, LOW);
                    uint16_t receivedData = vspi.transfer16(transferData);
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
        };
    }
};

void ADS8686S_SPI_Handler::writeRegister(uint8_t REGADDR, uint8_t DATA)
{
    if (!digitalRead(BUSY))
    {
        uint16_t transferData = ((0x80 | (REGADDR << 1)) << 8) | DATA;
        digitalWrite(CS, LOW);
        vspi.transfer16(transferData);
        digitalWrite(CS, HIGH);
    };
};

uint16_t ADS8686S_SPI_Handler::readRegister(uint8_t REGADDR)
{
    if (!digitalRead(BUSY))
    {
        uint16_t transferData = (REGADDR << 1) | 0x00;
        digitalWrite(CS, LOW);
        vspi.transfer16(transferData);
        uint16_t dataIn = vspi.transfer16(0x0000);
        digitalWrite(CS, HIGH);
        return dataIn;
    };
};

void ADS8686S_SPI_Handler::initiateSample(void) {

};
