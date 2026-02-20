/*
 * This file is part of the stm32-template project.
 *
 * Copyright (C) 2020 Johannes Huebner <dev@johanneshuebner.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file contains all parameters used in your project
 * See main.cpp on how to access them.
 * If a parameters unit is of format "0=Choice, 1=AnotherChoice" etc.
 * It will be displayed as a dropdown in the web interface
 * If it is a spot value, the decimal is translated to the name, i.e. 0 becomes "Choice"
 * If the enum values are powers of two, they will be displayed as flags, example
 * "0=None, 1=Flag1, 2=Flag2, 4=Flag3, 8=Flag4" and the value is 5.
 * It means that Flag1 and Flag3 are active -> Display "Flag1 | Flag3"
 *
 * Every parameter/value has a unique ID that must never change. This is used when loading parameters
 * from flash, so even across firmware versions saved parameters in flash can always be mapped
 * back to our list here. If a new value is added, it will receive its default value
 * because it will not be found in flash.
 * The unique ID is also used in the CAN module, to be able to recover the CAN map
 * no matter which firmware version saved it to flash.
 * Make sure to keep track of your ids and avoid duplicates. Also don't re-assign
 * IDs from deleted parameters because you will end up loading some random value
 * into your new parameter!
 * IDs are 16 bit, so 65535 is the maximum
 */

//Define a version string of your firmware here
#define VER 0.01.AK

/* Entries must be ordered as follows:
   1. Saveable parameters (id != 0)
   2. Temporary parameters (id = 0)
   3. Display values
 */
//Next param id (increase when adding new parameter!): 15
//Next value Id: 2182
/*      category     			name         	unit       min     	max     default id */
#define PARAM_LIST \
    PARAM_ENTRY(CAT_BMS,     	bmstype,      	TYPES,		0,     	3,      0,     	1 )\
    PARAM_ENTRY(CAT_BMS,     	numbmbs,     	"",       	1,      20,     4,      2 )\
    PARAM_ENTRY(CAT_BMS,     	balance,     	OFFON,     	0,      1,      0,      3 )\
	PARAM_ENTRY(CAT_BMS,     	Vbalance,     	"mV",     	3500,   4200,   3900,   4 )\
    PARAM_ENTRY(CAT_BMS,     	BattCap,     	"kWh",     	1,    	250,    22,     5 )\
    PARAM_ENTRY(CAT_BMS,     	CellVmax,     	"mV",      	3000, 	4200,   4150,   6 )\
    PARAM_ENTRY(CAT_BMS,     	CellVmin,     	"mV",      	2800, 	3500,   3200,   7 )\
	PARAM_ENTRY(CAT_BMS,     	IDCmax,     	"A",      	0, 		1500,   500,   	8 )\
	PARAM_ENTRY(CAT_BMS,     	IDCmin,     	"A",      	-1500, 	0,   	-500,   9 )\
    PARAM_ENTRY(CAT_BMS,     	CellTmax,     	"C",      	25, 	65,   	40,   	10)\
    PARAM_ENTRY(CAT_BMS,     	CellTmin,     	"C",      	-20, 	25,   	5,   	11)\
	PARAM_ENTRY(CAT_ALRM,    	VOffset,     	"mV",      	0, 		500,   	100,   	12)\
	PARAM_ENTRY(CAT_ALRM,    	Vdelta,     	"mV",      	0, 		500,   	100,   	13)\
	PARAM_ENTRY(CAT_ALRM,    	Vignore,     	"mV",      	0, 		1000,   500,   	14)\
	PARAM_ENTRY(CAT_COMM,    	CanCtrl,      	OFFON,     	0,      1,      0,   	15)\
	PARAM_ENTRY(CAT_COMM,    	NodeId,    		"",     	1,      63,     5,      16)\
	PARAM_ENTRY(CAT_SENS,    	ShuntType,   	SHNTYPE,   	0,      2,      0,      17)\
    PARAM_ENTRY(CAT_SENS,    	IsaInit,     	OFFON,     	0,      1,      0,      18)\
	PARAM_ENTRY(CAT_PWM,     	Tim3_Frequency,	FREQ,       3,      7,  	5,   	19)\
	PARAM_ENTRY(CAT_PWM,     	PWM3_CH3,     	OFFON,     	0,      1,      0,   	20)\
    PARAM_ENTRY(CAT_PWM,     	Tim3_3_DC,   	"",        	1,     	100, 	50,  	21)\
	PARAM_ENTRY(CAT_PWM,     	PWM3_CH4,     	OFFON,     	0,      1,      0,   	22)\
    PARAM_ENTRY(CAT_PWM,     	Tim3_4_DC,   	"",        	1,     	100, 	50,  	23)\
	PARAM_ENTRY(CAT_PWM,  	 	CH1_Frequency, 	FREQLow,    0,      2,  	1,   	24)\
	PARAM_ENTRY(CAT_PWM,  	 	LOW_CH1,     	OFFON,     	0,      1,      0,   	25)\
	PARAM_ENTRY(CAT_PWM,  	 	LOW_1_DC,   	"",        	1,     	100, 	50,  	26)\
	PARAM_ENTRY(CAT_PWM,  	 	CH2_Frequency, 	FREQLow,    0,      2,  	1,   	27)\
	PARAM_ENTRY(CAT_PWM,  	 	LOW_CH2,     	OFFON,     	0,      1,      0,   	28)\
	PARAM_ENTRY(CAT_PWM,  	 	LOW_2_DC,   	"",        	1,     	100, 	50,  	29)\
    VALUE_ENTRY(opmode,      	OPMODES,	2000 ) \
    VALUE_ENTRY(version,     	VERSTR, 	2001 ) \
	VALUE_ENTRY(IGN,   		 	OFFON,  	2002 ) \
    VALUE_ENTRY(CHG,   		 	OFFON,  	2003 ) \
	VALUE_ENTRY(GP1_din,   	 	OFFON,  	2004 ) \
    VALUE_ENTRY(GP2_din,     	OFFON,  	2005 ) \
	VALUE_ENTRY(GP1_ain,     	"Count",  	2051 ) \
	VALUE_ENTRY(uaux,        	"V",    	2006 ) \
	VALUE_ENTRY(CoolantPUMP, 	OFFON,  	2007 ) \
	VALUE_ENTRY(CoolantFAN,  	OFFON,  	2008 ) \
    VALUE_ENTRY(SOC,         	"%",    	2009 ) \
    VALUE_ENTRY(chargelim,   	"A",    	2010 ) \
    VALUE_ENTRY(dischargelim,	"A",    	2011 ) \
    VALUE_ENTRY(chargeVlim,  	"V",    	2012 ) \
    VALUE_ENTRY(dischargeVlim,	"V",   		2013 ) \
    VALUE_ENTRY(udc,         	"V",    	2014 ) \
	VALUE_ENTRY(idc,         	"A",    	2015 ) \
	VALUE_ENTRY(power,       	"kW",   	2016 ) \
	VALUE_ENTRY(udc1,        	"V",    	2017 ) \
    VALUE_ENTRY(udc2,        	"V",    	2018 ) \
    VALUE_ENTRY(udc3,        	"V",    	2019 ) \
	VALUE_ENTRY(KWh,         	"kwh",  	2020 ) \
    VALUE_ENTRY(AMPh,        	"Ah",   	2021 ) \
	VALUE_ENTRY(tmpaux,      	"°C",   	2022 ) \
	VALUE_ENTRY(PWM3CH3,       	OFFON, 		2023 ) \
	VALUE_ENTRY(PWM3CH3_DC,    	"Count", 	2024 ) \
	VALUE_ENTRY(PWM3CH4,       	OFFON, 		2025 ) \
	VALUE_ENTRY(PWM3CH4_DC,    	"Count", 	2026 ) \
	VALUE_ENTRY(LOWCH1,       	OFFON,		2027 ) \
	VALUE_ENTRY(LOWCH1_DC,    	"Count",	2028 ) \
	VALUE_ENTRY(LOWCH2,       	OFFON, 		2029 ) \
	VALUE_ENTRY(LOWCH2_DC,    	"Count",	2030 ) \
	VALUE_ENTRY(umin,        	"mV",   	2031 ) \
    VALUE_ENTRY(umax,        	"mV",   	2032 ) \
	VALUE_ENTRY(uavg,        	"mV",   	2033 ) \
	VALUE_ENTRY(deltaV,      	"mV",   	2034 ) \
    VALUE_ENTRY(TempMax,     	"°C",   	2035 ) \
    VALUE_ENTRY(TempMin,     	"°C",   	2036 ) \
	VALUE_ENTRY(Tempavg,     	"°C",   	2037 ) \
    VALUE_ENTRY(CellsPresent,  	"",   		2038 ) \
    VALUE_ENTRY(Chip1Cells,    	"",   		2039 ) \
    VALUE_ENTRY(Chip2Cells,    	"",   		2040 ) \
    VALUE_ENTRY(Chip3Cells,    	"",   		2041 ) \
    VALUE_ENTRY(Chip4Cells,    	"",   		2042 ) \
    VALUE_ENTRY(Chip5Cells,    	"",   		2043 ) \
    VALUE_ENTRY(Chip6Cells,    	"",   		2044 ) \
    VALUE_ENTRY(Chip7Cells,    	"",   		2045 ) \
    VALUE_ENTRY(Chip8Cells,    	"",   		2046 ) \
    VALUE_ENTRY(CellsBalancing,	"",   		2047 ) \
    VALUE_ENTRY(LoopCnt,      	"",    		2048 ) \
    VALUE_ENTRY(LoopState,    	"",    		2049 ) \
    VALUE_ENTRY(cpuload,     	"%",    	2050 ) \
    VALUE_ENTRY(u1,          	"mV",   	2101 ) \
    VALUE_ENTRY(u2,          	"mV",   	2102 ) \
    VALUE_ENTRY(u3,          	"mV",   	2103 ) \
    VALUE_ENTRY(u4,          	"mV",   	2104 ) \
    VALUE_ENTRY(u5,          	"mV",   	2105 ) \
    VALUE_ENTRY(u6,          	"mV",   	2106 ) \
    VALUE_ENTRY(u7,          	"mV",   	2107 ) \
    VALUE_ENTRY(u8,          	"mV",   	2108 ) \
    VALUE_ENTRY(u9,          	"mV",   	2109 ) \
    VALUE_ENTRY(u10,         	"mV",   	2110 ) \
    VALUE_ENTRY(u11,         	"mV",   	2111 ) \
    VALUE_ENTRY(u12,         	"mV",   	2112 ) \
    VALUE_ENTRY(u13,         	"mV",   	2113 ) \
    VALUE_ENTRY(u14,         	"mV",   	2114 ) \
    VALUE_ENTRY(u15,         	"mV",   	2115 ) \
    VALUE_ENTRY(u16,         	"mV",   	2116 ) \
    VALUE_ENTRY(u17,         	"mV",   	2117 ) \
    VALUE_ENTRY(u18,         	"mV",   	2118 ) \
    VALUE_ENTRY(u19,         	"mV",   	2119 ) \
    VALUE_ENTRY(u20,         	"mV",   	2120 ) \
    VALUE_ENTRY(u21,         	"mV",   	2121 ) \
    VALUE_ENTRY(u22,         	"mV",   	2122 ) \
    VALUE_ENTRY(u23,         	"mV",   	2123 ) \
    VALUE_ENTRY(u24,         	"mV",   	2124 ) \
    VALUE_ENTRY(u25,         	"mV",   	2125 ) \
    VALUE_ENTRY(u26,         	"mV",   	2126 ) \
    VALUE_ENTRY(u27,         	"mV",   	2127 ) \
    VALUE_ENTRY(u28,         	"mV",   	2128 ) \
    VALUE_ENTRY(u29,         	"mV",   	2129 ) \
    VALUE_ENTRY(u30,         	"mV",   	2130 ) \
    VALUE_ENTRY(u31,         	"mV",   	2131 ) \
    VALUE_ENTRY(u32,         	"mV",   	2132 ) \
    VALUE_ENTRY(u33,         	"mV",   	2133 ) \
    VALUE_ENTRY(u34,         	"mV",   	2134 ) \
    VALUE_ENTRY(u35,         	"mV",   	2135 ) \
    VALUE_ENTRY(u36,         	"mV",   	2136 ) \
    VALUE_ENTRY(u37,         	"mV",   	2137 ) \
    VALUE_ENTRY(u38,         	"mV",   	2138 ) \
    VALUE_ENTRY(u39,         	"mV",   	2139 ) \
    VALUE_ENTRY(u40,         	"mV",   	2140 ) \
    VALUE_ENTRY(u41,         	"mV",   	2141 ) \
    VALUE_ENTRY(u42,         	"mV",   	2142 ) \
    VALUE_ENTRY(u43,         	"mV",   	2143 ) \
    VALUE_ENTRY(u44,         	"mV",   	2144 ) \
    VALUE_ENTRY(u45,         	"mV",   	2145 ) \
    VALUE_ENTRY(u46,         	"mV",   	2146 ) \
    VALUE_ENTRY(u47,         	"mV",   	2147 ) \
    VALUE_ENTRY(u48,         	"mV",   	2148 ) \
    VALUE_ENTRY(u49,         	"mV",   	2149 ) \
    VALUE_ENTRY(u50,         	"mV",   	2150 ) \
    VALUE_ENTRY(u51,         	"mV",   	2151 ) \
    VALUE_ENTRY(u52,         	"mV",   	2152 ) \
    VALUE_ENTRY(u53,         	"mV",   	2153 ) \
    VALUE_ENTRY(u54,         	"mV",   	2154 ) \
    VALUE_ENTRY(u55,         	"mV",   	2155 ) \
    VALUE_ENTRY(u56,         	"mV",   	2156 ) \
    VALUE_ENTRY(u57,         	"mV",   	2157 ) \
    VALUE_ENTRY(u58,         	"mV",   	2158 ) \
    VALUE_ENTRY(u59,         	"mV",   	2159 ) \
    VALUE_ENTRY(u60,         	"mV",   	2160 ) \
    VALUE_ENTRY(u61,         	"mV",   	2161 ) \
    VALUE_ENTRY(u62,         	"mV",   	2162 ) \
    VALUE_ENTRY(u63,         	"mV",   	2163 ) \
    VALUE_ENTRY(u64,         	"mV",   	2164 ) \
    VALUE_ENTRY(u65,         	"mV",   	2165 ) \
    VALUE_ENTRY(u66,         	"mV",   	2166 ) \
    VALUE_ENTRY(u67,         	"mV",   	2167 ) \
    VALUE_ENTRY(u68,         	"mV",   	2168 ) \
    VALUE_ENTRY(u69,         	"mV",   	2169 ) \
    VALUE_ENTRY(u70,         	"mV",   	2170 ) \
    VALUE_ENTRY(u71,         	"mV",   	2171 ) \
    VALUE_ENTRY(u72,         	"mV",   	2172 ) \
    VALUE_ENTRY(u73,         	"mV",   	2173 ) \
    VALUE_ENTRY(u74,         	"mV",   	2174 ) \
    VALUE_ENTRY(u75,         	"mV",   	2175 ) \
    VALUE_ENTRY(u76,         	"mV",   	2176 ) \
    VALUE_ENTRY(u77,         	"mV",   	2177 ) \
    VALUE_ENTRY(u78,         	"mV",   	2178 ) \
    VALUE_ENTRY(u79,         	"mV",   	2179 ) \
    VALUE_ENTRY(u80,         	"mV",   	2180 ) \
    VALUE_ENTRY(u81,         	"mV",   	2181 ) \
    VALUE_ENTRY(u82,         	"mV",   	2182 ) \
    VALUE_ENTRY(u83,         	"mV",   	2183 ) \
    VALUE_ENTRY(u84,         	"mV",   	2184 ) \
    VALUE_ENTRY(u85,         	"mV",   	2185 ) \
    VALUE_ENTRY(u86,         	"mV",   	2186 ) \
    VALUE_ENTRY(u87,         	"mV",   	2187 ) \
    VALUE_ENTRY(u88,         	"mV",   	2188 ) \
    VALUE_ENTRY(u89,         	"mV",   	2189 ) \
    VALUE_ENTRY(u90,         	"mV",   	2190 ) \
    VALUE_ENTRY(u91,         	"mV",   	2191 ) \
    VALUE_ENTRY(u92,         	"mV",   	2192 ) \
    VALUE_ENTRY(u93,         	"mV",   	2193 ) \
    VALUE_ENTRY(u94,         	"mV",   	2194 ) \
    VALUE_ENTRY(u95,         	"mV",   	2195 ) \
    VALUE_ENTRY(u96,         	"mV",   	2196 ) \
    VALUE_ENTRY(u97,         	"mV",   	2197 ) \
    VALUE_ENTRY(u98,         	"mV",   	2198 ) \
    VALUE_ENTRY(u99,         	"mV",   	2199 ) \
    VALUE_ENTRY(u100,        	"mV",   	2200 ) \
    VALUE_ENTRY(u101,        	"mV",   	2201 ) \
    VALUE_ENTRY(u102,        	"mV",   	2202 ) \
    VALUE_ENTRY(u103,        	"mV",   	2203 ) \
    VALUE_ENTRY(u104,        	"mV",   	2204 ) \
    VALUE_ENTRY(u105,        	"mV",   	2205 ) \
    VALUE_ENTRY(u106,        	"mV",   	2206 ) \
    VALUE_ENTRY(u107,        	"mV",   	2207 ) \
    VALUE_ENTRY(u108,        	"mV",   	2208 ) \
    VALUE_ENTRY(u109,        	"mV",   	2209 ) \
    VALUE_ENTRY(u110,        	"mV",   	2210 ) \
    VALUE_ENTRY(u111,        	"mV",   	2211 ) \
    VALUE_ENTRY(u112,        	"mV",   	2212 ) \
    VALUE_ENTRY(u113,        	"mV",   	2213 ) \
	VALUE_ENTRY(u114,        	"mV",   	2214 ) \
    VALUE_ENTRY(u115,        	"mV",   	2215 ) \
    VALUE_ENTRY(u116,        	"mV",   	2216 ) \
    VALUE_ENTRY(u117,        	"mV",   	2217 ) \
    VALUE_ENTRY(u118,        	"mV",   	2218 ) \
    VALUE_ENTRY(u119,        	"mV",   	2219 ) \
    VALUE_ENTRY(u120,        	"mV",   	2220 ) \
    VALUE_ENTRY(Cellt1_0,    	"°C",   	2221 ) \
    VALUE_ENTRY(Cellt1_1,    	"°C",   	2222 ) \
    VALUE_ENTRY(Cellt2_0,    	"°C",   	2223 ) \
    VALUE_ENTRY(Cellt2_1,    	"°C",   	2224 ) \
    VALUE_ENTRY(Cellt3_0,    	"°C",   	2225 ) \
    VALUE_ENTRY(Cellt3_1,    	"°C",   	2226 ) \
    VALUE_ENTRY(Cellt4_0,    	"°C",   	2227 ) \
    VALUE_ENTRY(Cellt4_1,    	"°C",   	2228 ) \
    VALUE_ENTRY(Cellt5_0,    	"°C",   	2229 ) \
    VALUE_ENTRY(Cellt5_1,    	"°C",   	2230 ) \
    VALUE_ENTRY(Cellt6_0,    	"°C",   	2231 ) \
    VALUE_ENTRY(Cellt6_1,    	"°C",   	2232 ) \
    VALUE_ENTRY(Cellt7_0,    	"°C",   	2234 ) \
    VALUE_ENTRY(Cellt7_1,    	"°C",   	2235 ) \
	VALUE_ENTRY(Cellt8_0,    	"°C",   	2236 ) \
    VALUE_ENTRY(Cellt8_1,    	"°C",   	2237 ) \
    VALUE_ENTRY(Cellt9_0,    	"°C",   	2238 ) \
    VALUE_ENTRY(Cellt9_1,    	"°C",   	2239 ) \
    VALUE_ENTRY(Cellt10_0,    	"°C",   	2240 ) \
    VALUE_ENTRY(Cellt10_1,    	"°C",   	2241 ) \
    VALUE_ENTRY(Cellt11_0,    	"°C",   	2242 ) \
    VALUE_ENTRY(Cellt11_1,    	"°C",   	2243 ) \
    VALUE_ENTRY(Cellt12_0,    	"°C",   	2244 ) \
    VALUE_ENTRY(Cellt12_1,    	"°C",   	2245 ) \
    VALUE_ENTRY(Cellt13_0,    	"°C",   	2246 ) \
    VALUE_ENTRY(Cellt13_1,    	"°C",   	2247 ) \
    VALUE_ENTRY(Cellt14_0,    	"°C",   	2248 ) \
    VALUE_ENTRY(Cellt14_1,    	"°C",   	2249 ) \
    VALUE_ENTRY(Cellt15_0,    	"°C",   	2250 ) \
    VALUE_ENTRY(Cellt15_1,    	"°C",   	2251 ) \
	VALUE_ENTRY(Cellt16_0,    	"°C",   	2252 ) \
    VALUE_ENTRY(Cellt16_1,    	"°C",   	2253 ) \
    VALUE_ENTRY(Cellt17_0,    	"°C",   	2254 ) \
    VALUE_ENTRY(Cellt17_1,    	"°C",   	2255 ) \
    VALUE_ENTRY(Cellt18_0,    	"°C",   	2256 ) \
    VALUE_ENTRY(Cellt18_1,    	"°C",   	2257 ) \
    VALUE_ENTRY(Cellt19_0,    	"°C",   	2258 ) \
    VALUE_ENTRY(Cellt19_1,    	"°C",   	2259 ) \
    VALUE_ENTRY(Cellt20_0,    	"°C",   	2260 ) \
    VALUE_ENTRY(Cellt20_1,    	"°C",   	2261 ) \
    VALUE_ENTRY(Chipt1,      	"°C",   	2262 ) \
    VALUE_ENTRY(Chipt2,      	"°C",   	2263 ) \
    VALUE_ENTRY(Chipt3,      	"°C",   	2264 ) \
    VALUE_ENTRY(Chipt4,      	"°C",   	2265 ) \
    VALUE_ENTRY(Chipt5,      	"°C",   	2266 ) \
    VALUE_ENTRY(Chipt6,      	"°C",   	2267 ) \
    VALUE_ENTRY(Chipt7,      	"°C",   	2268 ) \
    VALUE_ENTRY(Chipt8,      	"°C",   	2269 ) \
    VALUE_ENTRY(ChipV1,       	"V",   		2270 ) \
    VALUE_ENTRY(ChipV2,       	"V",   		2271 ) \
    VALUE_ENTRY(ChipV3,       	"V",   		2272 ) \
    VALUE_ENTRY(ChipV4,       	"V",   		2273 ) \
    VALUE_ENTRY(ChipV5,       	"V",   		2274 ) \
    VALUE_ENTRY(ChipV6,       	"V",   		2275 ) \
    VALUE_ENTRY(ChipV7,       	"V",   		2278 ) \
    VALUE_ENTRY(ChipV8,       	"V",   		2279 ) 



/***** Enum String definitions *****/
#define OPMODES      "0=Off, 1=Run, 2=RunBalance, 3=Charge, 4=ChargeBalance"
#define SHNTYPE      "0=None, 1=ISA, 2=SBOX"
#define OFFON        "0=Off, 1=On"
#define BAL          "0=None, 1=Discharge"
#define TYPES        "0=Model_3, 1=Model_S, 2=BMW_PHEV, 3=Nissan_Leaf"
#define CAT_BMS      "Battery Management Settings"
#define CAT_ALRM     "Warning & Alarm Settings"
#define CAT_SENS     "Current Sensor setup"
#define CAT_COMM     "Communication"
#define CAT_PWM      "PWM Control"
#define FREQLow      "0=1Hz, 1=2Hz, 2=10Hz"
#define FREQ         "3=100Hz, 4=500Hz, 5=1kHz, 6=10kHz, 7=100kHz"

#define VERSTR STRINGIFY(4=VER)

/***** enums ******/

enum
{
    IDC_OFF, IDC_SINGLE, IDC_ISACAN
};


enum _modes
{
    MOD_OFF = 0,
    MOD_RUN,
	MOD_CHARGE,
    MOD_LAST
};

enum _types
{
    BMS_M3 = 0,
    BMS_TESLAS = 1,
    BMS_BMW = 2,
	BMS_LEAF = 3
};

//Generated enum-string for possible errors
extern const char* errorListString;
