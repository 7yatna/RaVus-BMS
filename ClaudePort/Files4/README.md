# Tesla Model S BMS Integration for RaVus BMS

Complete integration of Tesla Model S/X slave modules into the RaVus BMS framework.

## Overview

This integration adds support for Tesla Model S/X BMS slaves communicating via **USART1 @ 612500 baud** (PA9/PA10). All module data is published to the existing RaVus parameter system, making it accessible via CAN, terminal, and web interface.

## Hardware

- **USART1**: PA9 (TX), PA10 (RX) @ 612500 baud 8N1
- **Max modules**: 16 (configurable in ModelS.h)
- **Max cells**: 96 (16 modules × 6 cells each)

USART1 is already initialized in `hwinit.cpp` at the correct baud rate.

## Files to Replace/Update

### 1. **ModelS.h** → `/home/claude/RaVus-BMS-main/include/ModelS.h`
Complete rewrite with:
- `TeslaBMSModule` class (single module)
- `TeslaBMSManager` class (bus manager)
- Integration with `Param::` system

### 2. **ModelS.cpp** → `/home/claude/RaVus-BMS-main/src/ModelS.cpp`
Complete implementation:
- Module enumeration (RenumberModules, FindModules)
- Voltage/temperature reading with original Tesla scaling
- Cell balancing
- Parameter publishing (PublishToParams)

### 3. **BMSUtil.h** → `/home/claude/RaVus-BMS-main/include/BMSUtil.h`
Add Tesla protocol functions:
- `GenCRC()` - CRC-8 with polynomial 0x07
- `SendData()` - Send command to modules
- `GetReply()` - Receive response
- `SendDataWithReply()` - Send + receive with retry

### 4. **BMSUtil.cpp** → `/home/claude/RaVus-BMS-main/src/BMSUtil.cpp`
Add implementations of above functions.
Keep existing SOC calculation code intact.

### 5. **main.cpp** → `/home/claude/RaVus-BMS-main/src/main.cpp`
Add three things (see INTEGRATION_INSTRUCTIONS_main.txt):
- Include: `#include "ModelS.h"`
- Ms100Task(): Call `TeslaBMSManager::Task100Ms()` when `BMStype == BMS_TESLAS`
- main(): Call initialization functions when `BMStype == BMS_TESLAS`

## Integration Steps

1. **Copy files**:
   ```bash
   cp ModelS.h /home/claude/RaVus-BMS-main/include/
   cp ModelS.cpp /home/claude/RaVus-BMS-main/src/
   cp BMSUtil.h /home/claude/RaVus-BMS-main/include/   # Replace
   cp BMSUtil.cpp /home/claude/RaVus-BMS-main/src/     # Replace
   ```

2. **Edit main.cpp**:
   Follow `INTEGRATION_INSTRUCTIONS_main.txt` exactly.

3. **Build**:
   ```bash
   cd /home/claude/RaVus-BMS-main
   make
   ```

## Usage

### Initial Setup
1. Connect Tesla Model S modules to USART1 (PA9/PA10)
2. Set `bmstype` parameter to **1** (Model_S) via terminal
3. System will auto-enumerate modules on next power cycle

### Configuration Parameters
All configuration is done via existing RaVus parameters:

| Parameter | Description | Default | Notes |
|-----------|-------------|---------|-------|
| `bmstype` | BMS type selector | 0 | Set to **1** for Tesla Model S |
| `numbmbs` | Number of modules | 4 | Auto-detected, informational |
| `balance` | Enable balancing | 0 | 1 = enable |
| `Vbalance` | Balance threshold | 3900 | mV |

### Published Data
Every 100 ms, the following parameters are updated:

**Pack Aggregates:**
- `udc` — Pack voltage (V)
- `umin` — Lowest cell voltage (mV)
- `umax` — Highest cell voltage (mV)
- `deltaV` — Cell voltage spread (mV)
- `TempMin` — Lowest temperature (°C)
- `TempMax` — Highest temperature (°C)
- `Tempavg` — Average temperature (°C)
- `CellsPresent` — Total cell count

**Individual Cells:**
- `u1` through `u20` — First 20 cell voltages (mV)

### Terminal Commands
The Tesla BMS manager exposes these functions (can be called from terminal or custom code):

- `TeslaBMSManager::RenumberModules()` — Reset and re-enumerate all modules
- `TeslaBMSManager::FindModules()` — Scan for modules without reset
- `TeslaBMSManager::ClearFaults()` — Clear all module faults
- `TeslaBMSManager::GetPackVoltage()` — Get pack voltage (V)
- `TeslaBMSManager::GetNumModules()` — Get module count

## Technical Details

### Protocol
- **USART1** @ 612500 baud, 8N1
- **CRC-8** polynomial 0x07
- **Timeout**: 5 ms per byte
- **Retry**: 3 attempts on read failure

### Voltage Scaling (Original Tesla)
- Cell voltage: `raw16 * 0.000381493` V
- Module voltage: sum of 6 cells

### Temperature Scaling (Steinhart-Hart)
```cpp
tempTemp = (1.78 / ((raw16 + offset) / constant) - 3.57) * 1000
tempCalc = 1.0 / (A + B*ln(tempTemp) + C*ln³(tempTemp))
result = tempCalc - 273.15  // Celsius
```

### Cell Balancing
- Triggered when `balance == 1` AND `highCell > Vbalance` AND `delta > 40mV`
- Balances all cells above lowest cell voltage
- Duration: 50 seconds (hardcoded, can be parameterized)

### Scheduler Integration
Runs on **Ms100Task** (100 ms interval):
1. Read all module voltages/temps
2. Check balancing conditions
3. Start/stop balancing
4. Publish all data to parameters

## Differences from Original TeslaBMSV2

| Feature | Original | RaVus Integration |
|---------|----------|-------------------|
| USART | USART3 @ 612500 | USART1 @ 612500 |
| Timing | `millis()` | `uDelay()` from libopeninv |
| Logging | Serial console | None (use parameters) |
| Config | Flash EEPROM | Param system |
| SOC | Coulomb counting | BMSUtil::UpdateSOC() |
| CAN | Victron VE.Can | CanMap (RaVus) |
| Max modules | 62 | 16 (configurable) |

## Testing

1. **Check USART1 output**:
   - Connect logic analyzer to PA9
   - Should see 612500 baud traffic every 100ms

2. **Verify parameters**:
   - Terminal: `get umin umax deltaV`
   - Should show valid cell voltages

3. **Test balancing**:
   - Set `balance = 1`, `Vbalance = 3900`
   - Verify balancing activates when delta > 40mV

## Troubleshooting

**No modules found:**
- Check USART1 wiring (PA9=TX, PA10=RX)
- Verify 612500 baud is correct for your modules
- Try `TeslaBMSManager::RenumberModules()` from terminal

**CRC errors:**
- Check for noise on USART1 lines
- Verify ground connection between STM32 and modules
- Try shorter cables or twisted pair

**Incorrect voltages:**
- Scaling factors are hardcoded from original Tesla BMS
- Should be accurate ±1 mV
- Check `umin` and `umax` match module reality

## Files Included

- `ModelS.h` — Header file
- `ModelS.cpp` — Implementation
- `BMSUtil.h` — Updated header
- `BMSUtil.cpp` — Updated implementation
- `INTEGRATION_INSTRUCTIONS_main.txt` — main.cpp changes
- `PARAMETERS_INFO.txt` — Parameter reference
- `README.md` — This file

## License

GPL v3 (matches RaVus BMS license)
