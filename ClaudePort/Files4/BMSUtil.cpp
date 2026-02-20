/*
 * BMSUtil.cpp
 * 
 * Copyright (C) 2023 Tom de Bree, Damien Maguire
 * 
 * SOC calculation + Tesla BMS protocol functions
 */
#include "BMSUtil.h"
#include "delay.h"
#include <libopencm3/stm32/usart.h>

/* -----------------------------------------------------------------------
 * SOC calculation (existing code)
 * ----------------------------------------------------------------------- */
//voltage to state of charge                0%    10%   20%   30%   40%   50%   60%   70%   80%   90%   100%
uint16_t voltageToSoc[] = { 3300, 3400, 3450, 3500, 3560, 3600, 3700, 3800, 4000, 4100, 4200 };

uint16_t TempSOC = 0;
uint32_t NoCurCounter = 0;
uint32_t NoCurRun = 20;
float NoCurLim = 1.0;
float asDiff = 0;

void BMSUtil::UpdateSOC()
{
    TempSOC = Param::GetInt(Param::SOC);

    if(ABS(Param::GetFloat(Param::idc)) < NoCurLim)
    {
        NoCurCounter++;
    }
    else
    {
        NoCurCounter=0;
    }

    if(NoCurCounter > NoCurRun)
    {
        TempSOC=EstimateSocFromVoltage();
    }
    else
    {
        TempSOC = TempSOC + (100 * asDiff / (3600 * Param::GetInt(Param::BattCap)));
    }

    Param::SetInt(Param::SOC,TempSOC);
}

int BMSUtil::EstimateSocFromVoltage()
{
    float lowestVoltage = Param::GetFloat(Param::umin);
    int n = sizeof(voltageToSoc) / sizeof(voltageToSoc[0]);

    for (int i = 0; i < n; i++)
    {
        if (lowestVoltage < voltageToSoc[i])
        {
            if (i == 0) return 0;

            float soc = i * 10;
            float lutDiff = voltageToSoc[i] - voltageToSoc[i - 1];
            float valDiff = voltageToSoc[i] - lowestVoltage;
            //interpolate
            soc -= (valDiff / lutDiff) * 10;
            return soc;
        }
    }
    return 100;
}

/* -----------------------------------------------------------------------
 * Tesla BMS protocol (new code)
 * USART1 @ 612500 baud, initialized in hwinit.cpp
 * ----------------------------------------------------------------------- */
#define BMS_BYTE_TIMEOUT_US  5000  // 5 ms timeout per byte

uint8_t BMSUtil::GenCRC(uint8_t *input, int lenInput)
{
    uint8_t crc = 0;
    for (int x = 0; x < lenInput; x++) {
        crc ^= input[x];
        for (int i = 0; i < 8; i++)
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (crc << 1);
    }
    return crc;
}

void BMSUtil::SendData(uint8_t *data, uint8_t dataLen, bool isWrite)
{
    uint8_t orig     = data[0];
    uint8_t addrByte = data[0];
    if (isWrite) addrByte |= 1;

    /* Flush stale RX */
    while (USART_SR(USART1) & USART_SR_RXNE)
        (void)usart_recv(USART1);

    usart_send_blocking(USART1, addrByte);
    for (int i = 1; i < dataLen; i++)
        usart_send_blocking(USART1, data[i]);

    data[0] = addrByte;
    if (isWrite)
        usart_send_blocking(USART1, GenCRC(data, dataLen));

    data[0] = orig;
}

int BMSUtil::GetReply(uint8_t *data, int maxLen)
{
    int numBytes = 0;

    while (numBytes < maxLen) {
        uint32_t timeoutStart = 0;
        bool got = false;
        
        /* Simple polling with timeout (~5ms per byte) */
        for (uint32_t t = 0; t < BMS_BYTE_TIMEOUT_US; t++) {
            if (USART_SR(USART1) & USART_SR_RXNE) {
                data[numBytes] = (uint8_t)usart_recv(USART1);
                got = true;
                break;
            }
            uDelay(1);  // 1 µs delay from libopeninv/delay.h
        }
        
        if (!got) break;
        numBytes++;
    }

    /* Drain overflow */
    while (USART_SR(USART1) & USART_SR_RXNE)
        (void)usart_recv(USART1);

    return numBytes;
}

int BMSUtil::SendDataWithReply(uint8_t *data, uint8_t dataLen, bool isWrite,
                                uint8_t *retData, int retLen)
{
    int attempts = 1, returnedLength = 0;
    
    while (attempts < 4) {
        SendData(data, dataLen, isWrite);
        uDelay(2000 * ((retLen / 8) + 1));  // 2ms * frame size
        returnedLength = GetReply(retData, retLen);
        if (returnedLength == retLen) return returnedLength;
        attempts++;
    }
    
    return returnedLength;
}
