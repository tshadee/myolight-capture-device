#pragma once
#ifndef ADS8686S_SPI_HANDLER_H
#define ADS8686S_SPI_HANDLER_H

#define SPI_FREQ 8000000

#include <Arduino.h>
#include <SPI.h>

#include "ADS_REG_DEFAULTS.h"

static const uint8_t cfgArrayDepth = 10;
static const uint8_t rxBufferDepth = 28;
static const uint8_t txBufferDepth = 10;

class SPIClass;

class ADS8686S_SPI_Handler
{
   private:
    int spiCLK;
    uint16_t txBuffer[txBufferDepth];
    uint16_t rxBuffer[rxBufferDepth];
    uint16_t configArr[cfgArrayDepth];
    int SDI, SDO, CS, SCLK, RST, BUSY, CONVST;
    bool CRC(uint16_t* incomingData);
    void reset(float resetDuration);
    void writeRegister(uint8_t REGADDR, uint8_t DATA);
    uint16_t readRegister(uint8_t REGADDR);
    SPIClass& vspi;

   public:
    ADS8686S_SPI_Handler(int SDI, int SDO, int CS, int SCLK, int RST, int BUSY, int CONVST,
                         SPIClass& spiInstance);

    uint16_t getTransmitBuffer(int element);
    uint16_t getReceiveBuffer(int element);
    uint16_t getConfigArr(int element);
    uint16_t* getTransmitBuffer(void);
    uint16_t* getReceiveBuffer(void);
    uint16_t* getConfigArr(void);
    void SPI_start(void);
    void SPI_close(void);
    void SPI_transact(int mode);
    void configureADC(cfgProfile configProfile);
    void configureArr(enum ADCConfigModes);
    void initiateSample(void);
    bool verifyADSFunction(void);
};

#endif  // ADS8686S_SPI_HANDLER_H
