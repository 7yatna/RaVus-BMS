/*
 * ModelS.cpp — Tesla Model S/X BMS slave module communication
 *
 * Integrated with RaVus BMS parameter system.
 * Uses USART1 (PA9=TX, PA10=RX @ 612500 baud) initialized in hwinit.cpp
 * 
 * All data published to Param:: database in PublishToParams()
 */

#include "ModelS.h"
#include "BMSUtil.h"
#include "my_math.h"
#include "delay.h"
#include <string.h>
#include <libopencm3/stm32/usart.h>

// Static member initialization
TeslaBMSModule TeslaBMSManager::modules[MAX_MODULES + 1];
int   TeslaBMSManager::numFoundModules = 0;
float TeslaBMSManager::packVolt        = 0.0f;
float TeslaBMSManager::lowCellVolt     = 0.0f;
float TeslaBMSManager::highCellVolt    = 0.0f;
float TeslaBMSManager::avgTemp         = 0.0f;
float TeslaBMSManager::lowTemp         = 0.0f;
float TeslaBMSManager::highTemp        = 0.0f;
int   TeslaBMSManager::totalCells      = 0;
bool  TeslaBMSManager::initialized     = false;

/* -----------------------------------------------------------------------
 * TeslaBMSModule implementation
 * ----------------------------------------------------------------------- */
TeslaBMSModule::TeslaBMSModule()
{
    for (int i = 0; i < 6; i++) {
        cellVolt[i]        = 0.0f;
        lowestCellVolt[i]  = 5.0f;
        highestCellVolt[i] = 0.0f;
    }
    moduleVolt         = 0.0f;
    temperatures[0]    = 0.0f;
    temperatures[1]    = 0.0f;
    lowestTemperature  = 200.0f;
    highestTemperature = -100.0f;
    exists             = false;
    alerts             = 0;
    faults             = 0;
    COVFaults          = 0;
    CUVFaults          = 0;
    moduleAddress      = 0;
    scells             = 6;
    smiss              = 0;
}

void TeslaBMSModule::clearModule()
{
    for (int i = 0; i < 6; i++) cellVolt[i] = 0.0f;
    moduleVolt      = 0.0f;
    temperatures[0] = 0.0f;
    temperatures[1] = 0.0f;
}

void TeslaBMSModule::readStatus()
{
    uint8_t payload[3], buff[8];
    
    payload[0] = moduleAddress << 1;
    payload[1] = 0;  // REG_DEV_STATUS
    payload[2] = 0x01;
    BMSUtil::SendDataWithReply(payload, 3, false, buff, 4);
    
    payload[0] = moduleAddress << 1;
    payload[1] = 0x20;  // REG_ALERT_STATUS
    payload[2] = 0x04;
    BMSUtil::SendDataWithReply(payload, 3, false, buff, 7);
    alerts    = buff[3];
    faults    = buff[4];
    COVFaults = buff[5];
    CUVFaults = buff[6];
}

void TeslaBMSModule::stopBalance()
{
    uint8_t payload[3], buff[8];
    payload[0] = moduleAddress << 1;
    payload[1] = 0x32;  // REG_BAL_CTRL
    payload[2] = 0;
    BMSUtil::SendDataWithReply(payload, 3, true, buff, 4);
}

bool TeslaBMSModule::readModuleValues()
{
    uint8_t payload[4], buff[50];
    uint8_t calcCRC;
    int     retLen;
    
    payload[0] = moduleAddress << 1;
    
    readStatus();
    
    // Configure ADC
    payload[1] = 0x30;  // REG_ADC_CTRL
    payload[2] = 0b00111101;  // Auto mode, all inputs
    BMSUtil::SendDataWithReply(payload, 3, true, buff, 3);
    
    payload[1] = 0x31;  // REG_IO_CTRL
    payload[2] = 0b00000011;  // Enable temp measurement
    BMSUtil::SendDataWithReply(payload, 3, true, buff, 3);
    
    payload[1] = 0x34;  // REG_ADC_CONV
    payload[2] = 1;
    BMSUtil::SendDataWithReply(payload, 3, true, buff, 3);
    
    // Read data
    payload[1] = 0x01;  // REG_GPAI
    payload[2] = 0x12;  // 18 bytes
    retLen = BMSUtil::SendDataWithReply(payload, 3, false, buff, 22);
    
    calcCRC = BMSUtil::GenCRC(buff, retLen - 1);
    
    if ((retLen == 22) && (buff[21] == calcCRC))
    {
        if (buff[0] == (moduleAddress << 1) && buff[1] == 0x01 && buff[2] == 0x12)
        {
            // Cell voltages (original scaling factors preserved)
            for (int i = 0; i < 6; i++) {
                cellVolt[i] = (buff[5 + (i * 2)] * 256 + buff[6 + (i * 2)]) * 0.000381493f;
                if (lowestCellVolt[i]  > cellVolt[i] && cellVolt[i] >= 0.5f)
                    lowestCellVolt[i] = cellVolt[i];
                if (highestCellVolt[i] < cellVolt[i])
                    highestCellVolt[i] = cellVolt[i];
            }
            
            // Module voltage (sum of cells)
            moduleVolt = 0;
            for (int i = 0; i < 6; i++) moduleVolt += cellVolt[i];
            
            // Temperatures (Steinhart-Hart)
            float tempTemp = (1.78f / ((buff[17] * 256 + buff[18] + 2) / 33046.0f) - 3.57f) * 1000.0f;
            float tempCalc = 1.0f / (0.0007610373573f + (0.0002728524832f * logf(tempTemp)) +
                                     (powf(logf(tempTemp), 3) * 0.0000001022822735f));
            temperatures[0] = tempCalc - 273.15f;
            
            tempTemp = (1.78f / ((buff[19] * 256 + buff[20] + 9) / 33068.0f) - 3.57f) * 1000.0f;
            tempCalc = 1.0f / (0.0007610373573f + (0.0002728524832f * logf(tempTemp)) +
                              (powf(logf(tempTemp), 3) * 0.0000001022822735f));
            temperatures[1] = tempCalc - 273.15f;
            
            if (getLowTemp()  < lowestTemperature)  lowestTemperature  = getLowTemp();
            if (getHighTemp() > highestTemperature) highestTemperature = getHighTemp();
            
            return true;
        }
    }
    
    return false;
}

float TeslaBMSModule::getCellVoltage(int cell)
{
    if (cell < 0 || cell > 5) return 0.0f;
    return cellVolt[cell];
}

float TeslaBMSModule::getLowCellV()
{
    float lowVal = 10.0f;
    for (int i = 0; i < 6; i++)
        if (cellVolt[i] < lowVal && cellVolt[i] > 0.5f) lowVal = cellVolt[i];
    return lowVal;
}

float TeslaBMSModule::getHighCellV()
{
    float hiVal = 0.0f;
    for (int i = 0; i < 6; i++)
        if (cellVolt[i] > hiVal && cellVolt[i] < 4.5f) hiVal = cellVolt[i];
    return hiVal;
}

float TeslaBMSModule::getAverageV()
{
    int   x = 0;
    float avgVal = 0.0f;
    for (int i = 0; i < 6; i++) {
        if (cellVolt[i] > 0.5f && cellVolt[i] < 60.0f) {
            x++;
            avgVal += cellVolt[i];
        }
    }
    if (scells != x) {
        if (smiss > 2) scells = x;
        else           smiss++;
    } else {
        scells = x;
        smiss  = 0;
    }
    return (x > 0) ? avgVal / x : 0.0f;
}

int   TeslaBMSModule::getNumCells()       { return scells; }
float TeslaBMSModule::getModuleVoltage()  { return moduleVolt; }
float TeslaBMSModule::getTemperature(int temp)
{
    if (temp < 0 || temp > 1) return 0.0f;
    return temperatures[temp];
}

float TeslaBMSModule::getLowTemp()
{
    return (temperatures[0] < temperatures[1]) ? temperatures[0] : temperatures[1];
}

float TeslaBMSModule::getHighTemp()
{
    return (temperatures[0] < temperatures[1]) ? temperatures[1] : temperatures[0];
}

float TeslaBMSModule::getAvgTemp()
{
    return (temperatures[0] + temperatures[1]) * 0.5f;
}

uint8_t TeslaBMSModule::getFaults()    { return faults; }
uint8_t TeslaBMSModule::getAlerts()    { return alerts; }
uint8_t TeslaBMSModule::getCOVCells()  { return COVFaults; }
uint8_t TeslaBMSModule::getCUVCells()  { return CUVFaults; }
void    TeslaBMSModule::setAddress(int newAddr) { if (newAddr >= 0 && newAddr <= 0x3E) moduleAddress = (uint8_t)newAddr; }
int     TeslaBMSModule::getAddress()   { return moduleAddress; }
bool    TeslaBMSModule::isExisting()   { return exists; }
void    TeslaBMSModule::setExists(bool ex) { exists = ex; }

/* -----------------------------------------------------------------------
 * TeslaBMSManager implementation
 * ----------------------------------------------------------------------- */
void TeslaBMSManager::Init()
{
    for (int i = 1; i <= MAX_MODULES; i++) {
        modules[i].setExists(false);
        modules[i].setAddress(i);
    }
    initialized = true;
}

void TeslaBMSManager::SetupBoards()
{
    uint8_t payload[3], buff[10];
    int retLen;
    
    while (1) {
        payload[0] = 0; payload[1] = 0; payload[2] = 1;
        retLen = BMSUtil::SendDataWithReply(payload, 3, false, buff, 4);
        if (retLen == 4 && buff[0] == 0x80 && buff[1] == 0 && buff[2] == 1) {
            for (int y = 1; y <= MAX_MODULES; y++) {
                if (!modules[y].isExisting()) {
                    payload[0] = 0;
                    payload[1] = 0x3B;  // REG_ADDR_CTRL
                    payload[2] = y | 0x80;
                    BMSUtil::SendData(payload, 3, true);
                    uDelay(3 * 1000);
                    if (BMSUtil::GetReply(buff, 10) > 2) {
                        if (buff[0] == 0x81 && buff[1] == 0x3B && buff[2] == (y + 0x80)) {
                            modules[y].setExists(true);
                            numFoundModules++;
                        }
                    }
                    break;
                }
            }
        } else break;
    }
}

void TeslaBMSManager::FindModules()
{
    uint8_t payload[3], buff[8];
    numFoundModules = 0;
    
    for (int x = 1; x <= MAX_MODULES; x++) {
        modules[x].setExists(false);
        payload[0] = x << 1;
        payload[1] = 0;
        payload[2] = 1;
        BMSUtil::SendData(payload, 3, false);
        uDelay(20 * 1000);
        if (BMSUtil::GetReply(buff, 8) > 4) {
            if (buff[0] == (x << 1) && buff[1] == 0 && buff[2] == 1 && buff[4] > 0) {
                modules[x].setExists(true);
                numFoundModules++;
            }
        }
        uDelay(5 * 1000);
    }
}

void TeslaBMSManager::RenumberModules()
{
    uint8_t payload[3], buff[8];
    int attempts = 1;
    
    for (int y = 1; y <= MAX_MODULES; y++) {
        modules[y].setExists(false);
        numFoundModules = 0;
    }
    
    while (attempts < 3) {
        payload[0] = 0x3F << 1;
        payload[1] = 0x3C;
        payload[2] = 0xA5;
        BMSUtil::SendData(payload, 3, true);
        uDelay(100 * 1000);
        BMSUtil::GetReply(buff, 8);
        if (buff[0] == 0x7F && buff[1] == 0x3C && buff[2] == 0xA5 && buff[3] == 0x57) break;
        attempts++;
    }
    
    SetupBoards();
}

void TeslaBMSManager::ClearFaults()
{
    uint8_t payload[3], buff[8];
    
    payload[0] = 0x7F; payload[1] = 0x20; payload[2] = 0xFF;
    BMSUtil::SendDataWithReply(payload, 3, true, buff, 4);
    payload[2] = 0x00;
    BMSUtil::SendDataWithReply(payload, 3, true, buff, 4);
    
    payload[1] = 0x21; payload[2] = 0xFF;
    BMSUtil::SendDataWithReply(payload, 3, true, buff, 4);
    payload[2] = 0x00;
    BMSUtil::SendDataWithReply(payload, 3, true, buff, 4);
}

void TeslaBMSManager::StopBalancing()
{
    for (int x = 1; x <= MAX_MODULES; x++)
        if (modules[x].isExisting()) modules[x].stopBalance();
}

void TeslaBMSManager::BalanceCells()
{
    uint8_t payload[4], buff[30], balance;
    int balanceDuty = Param::GetInt(Param::balance) ? 50 : 0;
    
    for (int y = 1; y <= MAX_MODULES; y++) {
        if (!modules[y].isExisting()) continue;
        balance = 0;
        for (int i = 0; i < 6; i++)
            if (lowCellVolt < modules[y].getCellVoltage(i))
                balance |= (1 << i);
        
        if (balance != 0) {
            payload[0] = y << 1; payload[1] = 0x33; payload[2] = (uint8_t)balanceDuty;
            BMSUtil::SendData(payload, 3, true); uDelay(2 * 1000); BMSUtil::GetReply(buff, 30);
            
            payload[1] = 0x32; payload[2] = balance;
            BMSUtil::SendData(payload, 3, true); uDelay(2 * 1000); BMSUtil::GetReply(buff, 30);
        }
    }
}

void TeslaBMSManager::GetAllVoltTemp()
{
    packVolt = 0.0f;
    
    for (int x = 1; x <= MAX_MODULES; x++)
        if (modules[x].isExisting()) modules[x].stopBalance();
    
    osDelay(numFoundModules < 8 ? 200 : 50);
    
    totalCells = 0;
    for (int x = 1; x <= MAX_MODULES; x++) {
        if (!modules[x].isExisting()) continue;
        modules[x].readModuleValues();
        packVolt += modules[x].getModuleVoltage();
        totalCells += modules[x].getNumCells();
    }
    
    highCellVolt = 0.0f;
    lowCellVolt  = 5.0f;
    for (int x = 1; x <= MAX_MODULES; x++) {
        if (!modules[x].isExisting()) continue;
        if (modules[x].getHighCellV() > highCellVolt) highCellVolt = modules[x].getHighCellV();
        if (modules[x].getLowCellV()  < lowCellVolt)  lowCellVolt  = modules[x].getLowCellV();
    }
    
    // Temperature aggregates
    lowTemp  = 999.0f;
    highTemp = -999.0f;
    avgTemp  = 0.0f;
    int validTempCount = 0;
    
    for (int x = 1; x <= MAX_MODULES; x++) {
        if (!modules[x].isExisting()) continue;
        float modTemp = modules[x].getAvgTemp();
        if (modTemp > -70) {
            avgTemp += modTemp;
            if (modules[x].getHighTemp() > highTemp) highTemp = modules[x].getHighTemp();
            if (modules[x].getLowTemp()  < lowTemp)  lowTemp  = modules[x].getLowTemp();
            validTempCount++;
        }
    }
    if (validTempCount > 0) avgTemp /= validTempCount;
}

void TeslaBMSManager::PublishToParams()
{
    // Pack aggregates
    Param::SetFloat(Param::udc,      packVolt);
    Param::SetInt(Param::umin,       (int)(lowCellVolt * 1000.0f));
    Param::SetInt(Param::umax,       (int)(highCellVolt * 1000.0f));
    Param::SetInt(Param::deltaV,     (int)((highCellVolt - lowCellVolt) * 1000.0f));
    Param::SetInt(Param::TempMax,    (int)highTemp);
    Param::SetInt(Param::TempMin,    (int)lowTemp);
    Param::SetInt(Param::Tempavg,    (int)avgTemp);
    Param::SetInt(Param::CellsPresent, totalCells);
    
    // Publish first 20 cell voltages to u1..u20
    int cellIdx = 0;
    for (int m = 1; m <= MAX_MODULES && cellIdx < 20; m++) {
        if (!modules[m].isExisting()) continue;
        for (int c = 0; c < 6 && cellIdx < 20; c++) {
            int mv = (int)(modules[m].getCellVoltage(c) * 1000.0f);
            switch (cellIdx) {
                case 0:  Param::SetInt(Param::u1,  mv); break;
                case 1:  Param::SetInt(Param::u2,  mv); break;
                case 2:  Param::SetInt(Param::u3,  mv); break;
                case 3:  Param::SetInt(Param::u4,  mv); break;
                case 4:  Param::SetInt(Param::u5,  mv); break;
                case 5:  Param::SetInt(Param::u6,  mv); break;
                case 6:  Param::SetInt(Param::u7,  mv); break;
                case 7:  Param::SetInt(Param::u8,  mv); break;
                case 8:  Param::SetInt(Param::u9,  mv); break;
                case 9:  Param::SetInt(Param::u10, mv); break;
                case 10: Param::SetInt(Param::u11, mv); break;
                case 11: Param::SetInt(Param::u12, mv); break;
                case 12: Param::SetInt(Param::u13, mv); break;
                case 13: Param::SetInt(Param::u14, mv); break;
                case 14: Param::SetInt(Param::u15, mv); break;
                case 15: Param::SetInt(Param::u16, mv); break;
                case 16: Param::SetInt(Param::u17, mv); break;
                case 17: Param::SetInt(Param::u18, mv); break;
                case 18: Param::SetInt(Param::u19, mv); break;
                case 19: Param::SetInt(Param::u20, mv); break;
            }
            cellIdx++;
        }
    }
}

void TeslaBMSManager::Task100Ms()
{
    if (!initialized) return;
    
    GetAllVoltTemp();
    
    // Cell balancing decision
    int balanceVmV = Param::GetInt(Param::Vbalance);
    if (Param::GetInt(Param::balance) && 
        (highCellVolt * 1000.0f) > balanceVmV &&
        (highCellVolt - lowCellVolt) > 0.04f) {
        BalanceCells();
    } else {
        StopBalancing();
    }
    
    PublishToParams();
}

// Accessors
float TeslaBMSManager::GetPackVoltage()   { return packVolt; }
float TeslaBMSManager::GetAvgCellVolt()   { return packVolt / (totalCells > 0 ? totalCells : 1); }
float TeslaBMSManager::GetLowCellVolt()   { return lowCellVolt; }
float TeslaBMSManager::GetHighCellVolt()  { return highCellVolt; }
float TeslaBMSManager::GetAvgTemp()       { return avgTemp; }
float TeslaBMSManager::GetLowTemp()       { return lowTemp; }
float TeslaBMSManager::GetHighTemp()      { return highTemp; }
int   TeslaBMSManager::GetNumModules()    { return numFoundModules; }
int   TeslaBMSManager::GetTotalCells()    { return totalCells; }
