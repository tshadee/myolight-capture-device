#pragma once
#ifndef ADS8686S_SPI_HANDLER_H
#define ADS8686S_SPI_HANDLER_H

#define SPI_FREQ 20000000

#include <Arduino.h>
#include <SPI.h>

#include "ADS_REG_DEFAULTS.h"

static const uint8_t cfgArrayDepth = 10;
static const uint8_t rxBufferDepth = 8;
static const uint8_t txBufferDepth = 10;

class SPIClass;

class ADS8686S_SPI_Handler
{
   private:
    uint16_t txBuffer[txBufferDepth];
    uint16_t rxBuffer[rxBufferDepth];
    uint16_t configArr[cfgArrayDepth];
    int SDI, SDO, CS, SCLK, RST, BUSY, CONVST;
    // bool CRC(uint16_t* incomingData);
    // void reset(float resetDuration);
    SPIClass* vspi;

   public:
    ADS8686S_SPI_Handler(uint8_t SDI, uint8_t SDO, uint8_t CS, uint8_t SCLK, uint8_t RST,
                         uint8_t BUSY, uint8_t CONVST, SPIClass* spiInstance);

    uint16_t getTransmitBuffer(int element);
    uint16_t getReceiveBuffer(int element);
    uint16_t getConfigArr(int element);
    uint16_t* getTransmitBuffer(void);
    uint16_t* getReceiveBuffer(void);
    uint16_t* getConfigArr(void);

    void setConfigArr(uint16_t* data);
    void setTransmitBuffer(uint16_t* data);
    void clearReceiveBuffer(void);

    void writeRegister(uint8_t REGADDR, uint8_t DATA);
    uint16_t readRegister(uint8_t REGADDR);

    void SPI_open(void);
    void SPI_close(void);
    void configureADC(int configProfile);
    void configureArr(int configProfile);
    void initiateSample(void);
};

#endif  // ADS8686S_SPI_HANDLER_H
