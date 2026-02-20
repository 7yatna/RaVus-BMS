/*
 * BMSUtil.h
 * 
 * Copyright (C) 2023 Tom de Bree, Damien Maguire
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#ifndef BMSUtil_h
#define BMSUtil_h

#include "my_math.h"
#include <stdint.h>
#include "my_fp.h"
#include "params.h"

class BMSUtil
{
public:
    /* SOC calculation (existing) */
    static void UpdateSOC();
    static int  EstimateSocFromVoltage();
    
    /* Tesla BMS protocol (new) */
    static uint8_t GenCRC(uint8_t *input, int lenInput);
    static void    SendData(uint8_t *data, uint8_t dataLen, bool isWrite);
    static int     GetReply(uint8_t *data, int maxLen);
    static int     SendDataWithReply(uint8_t *data, uint8_t dataLen, bool isWrite, uint8_t *retData, int retLen);

private:
};

#endif
