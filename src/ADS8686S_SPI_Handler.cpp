
#include "ADS8686S_SPI_Handler.h"

#include "driver/gpio.h"

ADS8686S_SPI_Handler::ADS8686S_SPI_Handler(gpio_num_t CS, gpio_num_t RST, gpio_num_t BUSY,
                                           gpio_num_t CONVST, SPIClass* spiInstance,
                                           gpio_num_t MXS1, gpio_num_t MXS2, gpio_num_t MXEN)
    : CS(CS),
      RST(RST),
      CONVST(CONVST),
      BUSY(BUSY),
      vspi(spiInstance),
      MXS1(MXS1),
      MXS2(MXS2),
      MXEN(MXEN),
      configArr{CONFIG_DEFAULT_CFG,      RANGE_A1_DEFAULT_CFG,    RANGE_A2_DEFAULT_CFG,
                RANGE_B1_DEFAULT_CFG,    RANGE_B2_DEFAULT_CFG,    LPF_DEFAULT_CFG,
                SEQ_STACK_0_DEFAULT_CFG, SEQ_STACK_1_DEFAULT_CFG, SEQ_STACK_2_DEFAULT_CFG,
                SEQ_STACK_3_DEFAULT_CFG}

{
    std::fill(std::begin(rxBuffer), std::end(rxBuffer), 0);
    gpio_set_direction(CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONVST, GPIO_MODE_OUTPUT);
    gpio_set_direction(RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUSY, GPIO_MODE_INPUT);
    gpio_set_direction(MXS1, GPIO_MODE_OUTPUT);
    gpio_set_direction(MXS2, GPIO_MODE_OUTPUT);
    gpio_set_direction(MXEN, GPIO_MODE_OUTPUT);
    gpio_set_level(CS, HIGH);
    gpio_set_level(RST, LOW);
    gpio_set_level(CONVST, LOW);
    gpio_set_level(MXS1, LOW);
    gpio_set_level(MXS2, LOW);
    gpio_set_level(MXEN, HIGH);
    delayMicroseconds(500);
};

uint16_t ADS8686S_SPI_Handler::getConfigArr(int element) { return configArr[element]; };
uint16_t ADS8686S_SPI_Handler::getReceiveBuffer(int element) { return rxBuffer[element]; };
uint16_t* ADS8686S_SPI_Handler::getConfigArr(void) { return configArr; };
uint16_t* ADS8686S_SPI_Handler::getReceiveBuffer(void) { return rxBuffer; };

void ADS8686S_SPI_Handler::writeRegister(uint8_t REGADDR, uint8_t DATA)
{
    if (!gpio_get_level(BUSY))
    {
        uint16_t transferData = (0x8000 | (REGADDR << 9)) | DATA;
        vspi->beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE2));
        gpio_set_level(CS, LOW);
        vspi->transfer16(transferData);
        gpio_set_level(CS, HIGH);
        vspi->endTransaction();
    };
};

uint16_t ADS8686S_SPI_Handler::readRegister(uint8_t REGADDR)
{
    if (!digitalRead(BUSY))
    {
        uint16_t transferData = (REGADDR << 1) | 0x00;
        vspi->beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE2));
        gpio_set_level(CS, LOW);
        vspi->transfer16(transferData);
        uint16_t dataIn = vspi->transfer16(0x0000);
        gpio_set_level(CS, HIGH);
        vspi->endTransaction();
        return dataIn;
    }
    else
    {
        return 0xFFFF;
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

void ADS8686S_SPI_Handler::setReceiveBuffer(int element, uint16_t value)
{
    rxBuffer[element] = value;
}
// Call this function ONCE upon startup. ADC will undergo full reset and load in the
// configration profile. Make sure to set the cfgProfile in setConfigArr() before calling this
// function. Mode 2 SPI on FSPI line.
void ADS8686S_SPI_Handler::configureADC(void)
{
    delay(500);  // 0.5s delay in case this is called right after power supply enable
    gpio_set_level(RST, LOW);
    delay(50);  // FULL RESET. Hold RST low for 50 us (100 us safety margin)
    gpio_set_level(RST, HIGH);
    delay(50);  // From FREST to CS falling is 240 us minimum (500 for safety). 15ms until first
                // sample.
    if (!gpio_get_level(BUSY))
    {
        // writes to register (ADC output invalid)
        vspi->beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE2));
        for (int i = 0; i < cfgArrayDepth; i++)
        {
            gpio_set_level(CS, LOW);
            vspi->transfer16(configArr[i]);
            gpio_set_level(CS, HIGH);
            delayMicroseconds(1);
        };
        vspi->endTransaction();

        // register readback for verification (+1 cycle to readback)
        vspi->beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE2));
        gpio_set_level(CS, LOW);
        vspi->transfer16(configArr[0] & READBACK_MASK);
        gpio_set_level(CS, HIGH);
        vspi->endTransaction();

        for (int i = 1; i < cfgArrayDepth; i++)
        {
            uint16_t transferData = (configArr[i] & READBACK_MASK);
            vspi->beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE2));
            gpio_set_level(CS, LOW);
            uint16_t receivedData = (vspi->transfer16(transferData)) & READBACK_MASK_NO_ADDR;
            gpio_set_level(CS, HIGH);
            vspi->endTransaction();
            uint16_t expectedReadback = (configArr[i - 1]) & READBACK_MASK_NO_ADDR;
            if (receivedData != expectedReadback)
            {
                log_w("CFG Element   : %d :mismatch", i - 1);
                log_w("Received Data : %s", String(receivedData, BIN).c_str());
                log_w("Expected Data : %s", String(expectedReadback, BIN).c_str());
            }
            else
            {
                log_i("CFG Element   : %d :success", i - 1);
            };
        };

        vspi->beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE2));
        gpio_set_level(CS, LOW);
        uint16_t receivedData = (vspi->transfer16(0x0000)) & READBACK_MASK_NO_ADDR;
        gpio_set_level(CS, HIGH);
        vspi->endTransaction();
        uint16_t expectedReadback = (configArr[cfgArrayDepth - 1] & READBACK_MASK_NO_ADDR);
        if (receivedData != expectedReadback)
        {
            log_w("CFG Element   : %d :mismatch", cfgArrayDepth - 1);
            log_w("Received Data : %s", String(receivedData, BIN).c_str());
            log_w("Expected Data : %s", String(expectedReadback, BIN).c_str());
        }
        else
        {
            log_i("CFG Element   : %d :success", cfgArrayDepth - 1);
        };
    };

    gpio_set_level(CONVST, HIGH);  // pull CONVST high to initiate dummy conversion
    gpio_set_level(CONVST, LOW);   // this toggle takes like 1 us total
    while (gpio_get_level(BUSY));  // this is most likely already off
    vspi->transfer16(0x0000);      // you could do a dummy read but this is to set the SEQEN
                                   // pointer to the top of the stack
    clearReceiveBuffer();
    // ADC should be ready for use after this.
};

void ADS8686S_SPI_Handler::initiate4Sample(void)
{
    gpio_set_level(MXS2, LOW);  // channel 1 - AO = 0 ~ A1 = 0
    gpio_set_level(MXS1, LOW);
    delayMicroseconds(15);
    collectSamples(0);

    gpio_set_level(MXS2, LOW);  // channel 2 - A1 = 0 ~ A0 = 1
    gpio_set_level(MXS1, HIGH);
    delayMicroseconds(15);
    collectSamples(8);

    gpio_set_level(MXS2, HIGH);  // channel 3 - A1 = 1 ~ A0 = 0
    gpio_set_level(MXS1, LOW);
    delayMicroseconds(15);
    collectSamples(16);

    gpio_set_level(MXS2, HIGH);  // channel 4 - A1 = 1 ~ A0 = 1
    gpio_set_level(MXS1, HIGH);
    delayMicroseconds(15);
    collectSamples(24);
};

void ADS8686S_SPI_Handler::initiateSingleSample(int MXCH)
{
    switch (MXCH)
    {
        case (1):
            gpio_set_level(MXS2, LOW);
            gpio_set_level(MXS1, LOW);
            break;
        case (2):
            gpio_set_level(MXS2, LOW);
            gpio_set_level(MXS1, HIGH);
            break;
        case (3):
            gpio_set_level(MXS2, HIGH);
            gpio_set_level(MXS1, LOW);
            break;
        case (4):
            gpio_set_level(MXS2, HIGH);
            gpio_set_level(MXS1, HIGH);
            break;
        default:  // default to CH1
            gpio_set_level(MXS1, LOW);
            gpio_set_level(MXS2, LOW);
            break;
    }
    delayMicroseconds(15);  // TODO: check settling time
    collectSamples(0);
};

void ADS8686S_SPI_Handler::collectSamples(int startIndex)
{
    gpio_set_level(CONVST, HIGH);
    gpio_set_level(CONVST, LOW);
    while (gpio_get_level(BUSY));  // TODO: BUSY to CS falling minimum 20 ns.
    vspi->beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE2));
    gpio_set_level(CS, LOW);

    rxBuffer[startIndex] = vspi->transfer16(0x0000);
    rxBuffer[startIndex + 1] = vspi->transfer16(0x0000);
    rxBuffer[startIndex + 2] = vspi->transfer16(0x0000);
    rxBuffer[startIndex + 3] = vspi->transfer16(0x0000);
    rxBuffer[startIndex + 4] = vspi->transfer16(0x0000);
    rxBuffer[startIndex + 5] = vspi->transfer16(0x0000);
    rxBuffer[startIndex + 6] = vspi->transfer16(0x0000);
    rxBuffer[startIndex + 7] = vspi->transfer16(0x0000);

    gpio_set_level(CS, HIGH);
    vspi->endTransaction();
};