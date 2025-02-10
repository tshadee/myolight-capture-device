#pragma once
#ifndef ADS8686S_SPI_HANDLER_H
#define ADS8686S_SPI_HANDLER_H

#include <Arduino.h>
#include <SPI.h>

#include "COMMON_DEFS.h"

constexpr uint8_t cfgArrayDepth = 10;
constexpr uint8_t rxBufferDepth = 33;

class SPIClass;

class ADS8686S_SPI_Handler
{
   private:
    uint16_t rxBuffer[rxBufferDepth];
    uint16_t configArr[cfgArrayDepth];
    gpio_num_t BUSY;
    gpio_num_t RST, CS, CONVST;
    gpio_num_t MXS1, MXS2, MXEN;
    // bool CRC(uint16_t* incomingData);
    // void reset(float resetDuration);
    void configureSPI(void);
    void collectSamples(int startIndex);
    SPIClass* vspi;

   public:
    ADS8686S_SPI_Handler(gpio_num_t CS, gpio_num_t RST, gpio_num_t BUSY, gpio_num_t CONVST,
                         SPIClass* spiInstance, gpio_num_t MXS1, gpio_num_t MXS2, gpio_num_t MXEN);

    uint16_t getReceiveBuffer(int element);
    uint16_t getConfigArr(int element);
    uint16_t* getReceiveBuffer(void);
    uint16_t* getConfigArr(void);
    void setReceiveBuffer(int element, uint16_t value);

    void setConfigArr(uint16_t* data);
    void clearReceiveBuffer(void);

    void writeRegister(uint8_t REGADDR, uint8_t DATA);
    uint16_t readRegister(uint8_t REGADDR);
    void configureADC(void);
    void initiate4Sample(void);
    void initiateSingleSample(int MXCH);
};

#endif  // ADS8686S_SPI_HANDLER_H
