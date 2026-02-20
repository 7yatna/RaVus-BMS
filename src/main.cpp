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
#include <stdint.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/desig.h>
#include "stm32_can.h"
#include "canmap.h"
#include "cansdo.h"
#include "terminal.h"
#include "params.h"
#include "hwdefs.h"
#include "digio.h"
#include "hwinit.h"
#include "anain.h"
#include "param_save.h"
#include "my_math.h"
#include "errormessage.h"
#include "printf.h"
#include "stm32scheduler.h"
#include "terminalcommands.h"
#include "my_string.h"
#include "BatMan.h"
#include "leafbms.h"
#include "ModelS.h"
#include "BMSUtil.h"
#include "isa_shunt.h"
#include "bmw_sbox.h"
#define PRINT_JSON 0


			   
extern "C" void __cxa_pure_virtual()
{
    while (1);
}

static Stm32Scheduler* scheduler;
static CanHardware* can;
static CanMap* canMap;
static CanSdo* canSdo;
static uint8_t BMStype;
int uauxGain = 222;	
uint8_t Gcount = 0x00;
float SOCVal = 0;

//sample 10 ms task
static void Ms10Task(void)
{
    //Set timestamp of error message
    ErrorMessage::SetTime(rtc_get_counter_val());
	switch (Param::GetInt(Param::LOW_CH1))
	{
		case 1:
			{
				Param::SetInt(Param::LOWCH1, 1);
				Param::SetInt(Param::LOWCH1_DC, Param::GetInt(Param::LOW_1_DC));
				switch (Param::GetInt(Param::CH1_Frequency))
					{
						case 0:
							CH1Low1Hz();
							break;
						case 1:
							CH1Low2Hz();
							break;
						case 2:
							CH1Low10Hz();
							break;
					default:
						
					break;
					}
			}
			break;
		default:
			Param::SetInt(Param::LOWCH1, 0);
			Param::SetInt(Param::LOWCH1_DC, 0);
			DigIo::LOW1.Clear();
		break;
	}
	switch (Param::GetInt(Param::LOW_CH2))
	{
		case 1:
			{
				Param::SetInt(Param::LOWCH2, 1);
				Param::SetInt(Param::LOWCH2_DC, Param::GetInt(Param::LOW_2_DC));
				switch (Param::GetInt(Param::CH2_Frequency))
					{
						case 0:
							CH2Low1Hz();
							break;
						case 1:
							CH2Low2Hz();
							break;
						case 2:
							CH2Low10Hz();
							break;
					default:
						//Handle general parameter changes here. Add paramNum labels for handling specific parameters
					break;
					}
			}
			break;
		default:
			Param::SetInt(Param::LOWCH2, 0);
			Param::SetInt(Param::LOWCH2_DC, 0);
			DigIo::LOW2.Clear();
		break;
	}
	ProcessUdc();
}

	
//sample 100ms task
static void Ms100Task(void)
{
    DigIo::LED_ACT.Toggle();
    iwdg_reset();
	Param::SetInt(Param::IGN, DigIo::IGN.Get());
	Param::SetInt(Param::CHG, DigIo::CHG.Get());
	Param::SetFloat(Param::GP1_ain, (float)AnaIn::Ain.Get());
	Param::SetInt(Param::GP1_din, DigIo::DIN1.Get());
	Param::SetInt(Param::GP2_din, DigIo::DIN2.Get());
	if(DigIo::DIN1.Get() || (Param::GetInt(Param::opmode) == 1)) 
	{
		DigIo::PUMP.Set(); 
		Param::SetInt(Param::CoolantPUMP, 1);
	}
	else 
	{
		DigIo::PUMP.Clear();
		Param::SetInt(Param::CoolantPUMP, 0);
	}
	if((Param::GetInt(Param::opmode) == 1) && (DigIo::DIN2.Get() || (Param::GetInt(Param::TempMax) >= 35))) 
	{
		DigIo::FAN.Set();
		Param::SetInt(Param::CoolantFAN, 1);
	}
	else 
	{
		DigIo::FAN.Clear();
		Param::SetInt(Param::CoolantFAN, 0);
	}
    float cpuLoad = scheduler->GetCpuLoad();
    Param::SetFloat(Param::cpuload, cpuLoad / 10);
	/*
	if(Param::GetInt(Param::ShuntType) != 0)//Do not do any SOC calcs
    {
        CalcSOC();
    }
	*/
	
//!!! to change to BMS class with selectable types under it to clean up code and simplify interactions//
    if(BMStype == BMS_M3)
    {
        BATMan::loop();
    }
	/*
	else if(BMStype == BMS_TESLAS)
    {
        BATMan::loop();
    }
	
	else if(BMStype == BMS_BMW)
    {
        //BATMan::loop();
    }
	
	else if(BMStype == BMS_LEAF)
    {
        //BATMan::loop();
    }
    
	*/
    
///////////////
    BMSUtil::UpdateSOC();
    canMap->SendAll();
	Can_Tasks();
	int32_t IsaTemp=ISA::Temperature;
    Param::SetInt(Param::tmpaux,IsaTemp);
}

static void Ms200Task(void)
{
	LoadValues();
	tim3_setup();
}

void ProcessUdc()
{
  
    if (Param::GetInt(Param::ShuntType) == 1)//ISA shunt
    {
        float udc1 = ((float)ISA::Voltage)/1000;//get voltage from isa sensor and post to parameter database
        Param::SetFloat(Param::udc1, udc1);
        float udc2 = ((float)ISA::Voltage2)/1000;//get voltage from isa sensor and post to parameter database
        Param::SetFloat(Param::udc2, udc2);
        float udc3 = ((float)ISA::Voltage3)/1000;//get voltage from isa sensor and post to parameter database
        Param::SetFloat(Param::udc3, udc3);
        float idc = ((float)ISA::Amperes)/1000;//get current from isa sensor and post to parameter database
        Param::SetFloat(Param::idc, idc);
        float kw = ((float)ISA::KW)/1000;//get power from isa sensor and post to parameter database
        Param::SetFloat(Param::power, kw);
        float kwh = ((float)ISA::KWh)/1000;//get kwh from isa sensor and post to parameter database
        Param::SetFloat(Param::KWh, kwh);
        float Amph = ((float)ISA::Ah)/3600;//get Ah from isa sensor and post to parameter database
        Param::SetFloat(Param::AMPh, Amph);
    }
    else if (Param::GetInt(Param::ShuntType) == 2)//BMW Sbox
    {
        float udc = ((float)SBOX::Voltage2)/1000;//get output voltage from sbox sensor and post to parameter database
        Param::SetFloat(Param::udc1, udc);
        float udc2 = ((float)SBOX::Voltage)/1000;//get battery voltage from sbox sensor and post to parameter database
        Param::SetFloat(Param::udc2, udc2);
        float udc3 = 0;//((float)ISA::Voltage3)/1000;//get voltage from isa sensor and post to parameter database
        Param::SetFloat(Param::udc3, udc3);
        float idc = ((float)SBOX::Amperes)/1000;//get current from sbox sensor and post to parameter database
        Param::SetFloat(Param::idc, idc);
        float kw = (udc*idc)/1000;//get power from isa sensor and post to parameter database
        Param::SetFloat(Param::power, kw);
    }

    Param::SetFloat(Param::uaux, ((float)AnaIn::Vsense.Get()) / uauxGain);
}
void Can_Tasks()
{
	
	uint8_t bytes[8];
    bytes[0]= 0x00;
    bytes[1]= 0x00;
	bytes[2]= Gcount;
    bytes[3]= 0x0A;
    
    can->Send(0x1D2, bytes, 4); //Send on CAN1	
	
	Gcount = Gcount + 0x01;
   
   if (Gcount==0xFF)
   {
      Gcount=0x00;
   }
}

void DecodeCAN(int id, uint32_t* data)
{
	uint8_t* bytes = (uint8_t*)data;
	switch (id)
	{
		case 0x1AE:
			Param::SetInt(Param::opmode, bytes[0]);
			break;
	   default:
			break;
	}
			
}
static void SetCanFilters()
{
	if (Param::GetInt(Param::ShuntType) == 1)  ISA::RegisterCanMessages(can);//select isa shunt
	if (Param::GetInt(Param::ShuntType) == 2)  SBOX::RegisterCanMessages(can);//select bmw sbox
	if (Param::GetInt(Param::bmstype) == 3)  LeafBMS::RegisterCanMessages(can);//select bmw sbox
	can->RegisterUserMessage(0x605); //Can SDO
	can->RegisterUserMessage(0x1AE); //OI Control Message
}
	
static bool CanCallback(uint32_t id, uint32_t data[2], uint8_t dlc) //This is where we go when a defined CAN message is received.
{
    dlc = dlc;
	if (Param::GetInt(Param::CanCtrl)) DecodeCAN(id,data);
	if (Param::GetInt(Param::ShuntType) == 1) ISA::DecodeCAN(id, data);	
	if (Param::GetInt(Param::ShuntType) == 2) SBOX::DecodeCAN(id, data);
	if (Param::GetInt(Param::bmstype) == 3)  LeafBMS::DecodeCAN(id, data);
	return false;
}

/** This function is called when the user changes a parameter */


void Param::Change(Param::PARAM_NUM paramNum)
{
    switch (paramNum)
    {
		case Param::NodeId:
			canSdo->SetNodeId(Param::GetInt(Param::NodeId));
			break; 
		case Param::Tim3_Frequency:
		case PWM3_CH3:
		case Param::Tim3_3_DC:
		case PWM3_CH4:
		case Param::Tim3_4_DC:
			tim3_setup();
			break;
		case Param::bmstype:
		case Param::ShuntType:
		case Param::CanCtrl:
			SetCanFilters();
			break;
    default:
        //Handle general parameter changes here. Add paramNum labels for handling specific parameters
        break;
    }
}

//Whichever timer(s) you use for the scheduler, you have to
//implement their ISRs here and call into the respective scheduler
extern "C" void tim2_isr(void)
{
    scheduler->Run();
}

extern "C" int main(void)
{
    extern const TERM_CMD termCmds[];

    clock_setup(); //Must always come first
    rtc_setup();
    ANA_IN_CONFIGURE(ANA_IN_LIST);
    DIG_IO_CONFIGURE(DIG_IO_LIST);
    AnaIn::Start(); //Starts background ADC conversion via DMA
    write_bootloader_pininit(); //Instructs boot loader to initialize certain pins
	gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, AFIO_MAPR_CAN1_REMAP_PORTB);//Remap CAN pins to Portb alt funcs.
    nvic_setup(); //Set up some interrupts
    parm_load(); //Load stored parameters
    spi1_setup();// SPI1 for Model 3 BMB modules
	tim3_setup();
    usart1_setup();//Usart 1 for Model S / X slaves
    Stm32Scheduler s(TIM2); //We never exit main so it's ok to put it on stack
    scheduler = &s;

    //Initialize CAN1, including interrupts. Clock must be enabled in clock_setup()
    Stm32Can c(CAN1, CanHardware::Baud500,true);
    FunctionPointerCallback cb(CanCallback, SetCanFilters);
	CanMap cm(&c);
	CanSdo sdo(&c, &cm);
	sdo.SetNodeId(Param::GetInt(Param::NodeId));
//store a pointer for easier access
	can = &c;
    canMap = &cm;
    canSdo = &sdo;
	c.AddCallback(&cb);
    Terminal t(USART3, termCmds);
    TerminalCommands::SetCanMap(canMap);
    BATMan::BatStart();
    s.AddTask(Ms10Task, 10);
    s.AddTask(Ms100Task, 100);
	s.AddTask(Ms200Task, 200);
	
	if(Param::GetInt(Param::IsaInit)==1) ISA::initialize(can);//only call this once if a new sensor is fitted.
	Param::SetInt(Param::opmode, 0);//always off at startup


	if(BMStype == BMS_M3)
    {
        BATMan::BatStart();
    }
	/*
	else if(BMStype == BMS_TESLAS)
    {
        //BATMan::BatStart();
		usart_send_blocking(USART1, 'S');
    }
	
	else if(BMStype == BMS_BMW)
    {
       //BATMan::BatStart();
    }
	
	else if(BMStype == BMS_LEAF)
    {
        //BATMan::BatStart();
    }
    */
    Param::SetInt(Param::version, 4);
    Param::Change(Param::PARAM_LAST); //Call callback one for general parameter propagation
	SetCanFilters();
	if (Param::GetInt(Param::CanCtrl) == 0)
	{
		Param::SetInt(Param::opmode, 0);
	}
    while(1)
    {
        char c = 0;
        t.Run();
        if (sdo.GetPrintRequest() == PRINT_JSON)
        {
            TerminalCommands::PrintParamsJson(&sdo, &c);
        }
    }

    return 0;
}

