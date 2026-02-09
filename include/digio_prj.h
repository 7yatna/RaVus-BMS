#ifndef PinMode_PRJ_H_INCLUDED
#define PinMode_PRJ_H_INCLUDED

#include "hwdefs.h"

/* Here you specify generic IO pins, i.e. digital input or outputs.
 * Inputs can be floating (INPUT_FLT), have a 30k pull-up (INPUT_PU)
 * or pull-down (INPUT_PD) or be an output (OUTPUT)
*/

#define DIG_IO_LIST \
    DIG_IO_ENTRY(LED_ACT,      GPIOC, GPIO13, PinMode::OUTPUT)   \
    DIG_IO_ENTRY(BMAN_EN,      GPIOB, GPIO12, PinMode::OUTPUT)   \
	DIG_IO_ENTRY(BMAN_CS,      GPIOA, GPIO4,  PinMode::OUTPUT)   \
    DIG_IO_ENTRY(NEG,          GPIOA, GPIO2,  PinMode::OUTPUT)   \
    DIG_IO_ENTRY(POS,          GPIOA, GPIO3,  PinMode::OUTPUT)   \
	DIG_IO_ENTRY(PUMP,         GPIOB, GPIO5,  PinMode::OUTPUT)   \
	DIG_IO_ENTRY(FAN,          GPIOB, GPIO6,  PinMode::OUTPUT)   \
	DIG_IO_ENTRY(LOW1,         GPIOB, GPIO3,  PinMode::OUTPUT)   \
	DIG_IO_ENTRY(LOW2,         GPIOB, GPIO4,  PinMode::OUTPUT)   \
	DIG_IO_ENTRY(TIM3CH3,      GPIOB, GPIO0,  PinMode::OUTPUT)   \
	DIG_IO_ENTRY(TIM3CH4,      GPIOB, GPIO1,  PinMode::OUTPUT)   \
	DIG_IO_ENTRY(IGN,          GPIOB, GPIO13, PinMode::INPUT_FLT)\
    DIG_IO_ENTRY(CHG,          GPIOB, GPIO14, PinMode::INPUT_FLT)\
    DIG_IO_ENTRY(DIN1,         GPIOA, GPIO11, PinMode::INPUT_FLT)\
    DIG_IO_ENTRY(DIN2,         GPIOA, GPIO12, PinMode::INPUT_FLT)\

#endif // PinMode_PRJ_H_INCLUDED
