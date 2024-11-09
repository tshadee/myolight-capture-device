#pragma once
#ifndef ADS8686S_SPI_HANDLER_H
#define ADS8686S_SPI_HANDLER_H

#include <Arduino.h>
#include <SPI.h>

#include "ADS_REG_DEFAULTS.h"

constexpr uint8_t cfgArrayDepth = 10;
constexpr uint8_t rxBufferDepth = 8;

class SPIClass;

class ADS8686S_SPI_Handler
{
   private:
    uint16_t rxBuffer[rxBufferDepth];
    uint16_t configArr[cfgArrayDepth];
    uint8_t CS, RST, BUSY, CONVST;
    // bool CRC(uint16_t* incomingData);
    // void reset(float resetDuration);

    SPIClass* vspi;

   public:
    ADS8686S_SPI_Handler(uint8_t CS, uint8_t RST, uint8_t BUSY, uint8_t CONVST,
                         SPIClass* spiInstance);

    uint16_t getReceiveBuffer(int element);
    uint16_t getConfigArr(int element);
    uint16_t* getReceiveBuffer(void);
    uint16_t* getConfigArr(void);

    void setConfigArr(uint16_t* data);
    void clearReceiveBuffer(void);

    void writeRegister(uint8_t REGADDR, uint8_t DATA);
    uint16_t readRegister(uint8_t REGADDR);
    void configureADC(void);
    void initiateSample(void);
};

#endif  // ADS8686S_SPI_HANDLER_H
