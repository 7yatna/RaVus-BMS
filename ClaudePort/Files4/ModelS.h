#ifndef MODSX_h
#define MODSX_h

/*  This library supports USART comms with the Model S and X slave boards
 *  
 *  Uses USART1 (initialized in hwinit.cpp)
 *  Protocol: 612500 baud, 8N1
 *  
 *  Integration with RaVus BMS parameter system.
 *  All module data published to Param:: database.
 */

#include <stdint.h>
#include <stdbool.h>
#include "params.h"

#define MAX_MODULES 16  // Maximum Tesla BMS modules supported

class TeslaBMSModule
{
public:
    TeslaBMSModule();
    
    void readStatus();
    bool readModuleValues();
    void stopBalance();
    void clearModule();
    
    float getCellVoltage(int cell);
    float getLowCellV();
    float getHighCellV();
    float getAverageV();
    float getModuleVoltage();
    
    float getTemperature(int temp);
    float getLowTemp();
    float getHighTemp();
    float getAvgTemp();
    
    uint8_t getFaults();
    uint8_t getAlerts();
    uint8_t getCOVCells();
    uint8_t getCUVCells();
    
    void setAddress(int newAddr);
    int  getAddress();
    bool isExisting();
    void setExists(bool ex);
    int  getNumCells();

private:
    float    cellVolt[6];
    float    lowestCellVolt[6];
    float    highestCellVolt[6];
    float    moduleVolt;
    float    temperatures[2];
    float    lowestTemperature;
    float    highestTemperature;
    bool     exists;
    uint8_t  alerts;
    uint8_t  faults;
    uint8_t  COVFaults;
    uint8_t  CUVFaults;
    uint8_t  moduleAddress;
    int      scells;  // Actual cell count
    int      smiss;   // Miss counter for cell count stability
};

class TeslaBMSManager
{
public:
    static void Init();
    static void Task100Ms();  // Called from scheduler
    
    /* Bus management */
    static void RenumberModules();
    static void FindModules();
    static void ClearFaults();
    
    /* Data acquisition */
    static void GetAllVoltTemp();
    
    /* Cell balancing */
    static void BalanceCells();
    static void StopBalancing();
    
    /* Pack readings → published to Param:: */
    static float GetPackVoltage();
    static float GetAvgCellVolt();
    static float GetLowCellVolt();
    static float GetHighCellVolt();
    static float GetAvgTemp();
    static float GetLowTemp();
    static float GetHighTemp();
    static int   GetNumModules();
    static int   GetTotalCells();
    
private:
    static TeslaBMSModule modules[MAX_MODULES + 1];  // index 1..MAX_MODULES
    static int   numFoundModules;
    static float packVolt;
    static float lowCellVolt;
    static float highCellVolt;
    static float avgTemp;
    static float lowTemp;
    static float highTemp;
    static int   totalCells;
    static bool  initialized;
    
    static void SetupBoards();
    static void PublishToParams();
};

#endif /* MODSX_h */
