/* ###*B*###
 * Erika Enterprise, version 3
 * 
 * Copyright (C) 2017 - 2019 Evidence s.r.l.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License, version 2, for more details.
 * 
 * You should have received a copy of the GNU General Public License,
 * version 2, along with this program; if not, see
 * <www.gnu.org/licenses/old-licenses/gpl-2.0.html >.
 * 
 * This program is distributed to you subject to the following
 * clarifications and special exceptions to the GNU General Public
 * License, version 2.
 * 
 * THIRD PARTIES' MATERIALS
 * 
 * Certain materials included in this library are provided by third
 * parties under licenses other than the GNU General Public License. You
 * may only use, copy, link to, modify and redistribute this library
 * following the terms of license indicated below for third parties'
 * materials.
 * 
 * In case you make modified versions of this library which still include
 * said third parties' materials, you are obligated to grant this special
 * exception.
 * 
 * The complete list of Third party materials allowed with ERIKA
 * Enterprise version 3, together with the terms and conditions of each
 * license, is present in the file THIRDPARTY.TXT in the root of the
 * project.
 * ###*E*### */

/** \file	code.cpp
 *  \brief	Startup section
 *
 *  This file contains the code of main application for Erika Enterprise.
 *
 *  \author
 *  \date
 */

/* ERIKA Enterprise. */
#include "ee.h"

/* Arduino SDK. */
#include "Arduino.h"

/* Hardware Pins */
#include "hwpins.h"


extern "C" {

/* External Functions */
extern void serial_print(char const * msg);



/* Local Variables */
boolean   volatile stk_wrong = false;
OsEE_addr volatile old_sp;
AppModeType startupmode;					/* OSEE - Type defined AppMode from OSEE */

/* Stack Pointers */
OsEE_addr volatile main_sp;

/* Macro for OSEE Debugging only */
#define OSEE_BREAK_POINT()  do {                                    \
    cli();                                                          \
    serial_print("Test Failed!!!, line:" OSEE_S(__LINE__) " \r\n"); \
    while ( 1 ) {                                                   \
      if (serialEventRun) serialEventRun();                         \
    }                                                               \
  } while ( 0 )


void StartupHook(void)
{  									/* User defined pin declaration */
	/* Empty Functions */
}


void idle_hook(void)
{
  uint32_t counter = 0;
  OsEE_addr volatile curr_sp = osEE_get_SP();

  if ( main_sp == 0 ) {
    main_sp = curr_sp;
  } else if ( main_sp != curr_sp ) {
    OSEE_BREAK_POINT();
  }

  cli();
  if (serialEventRun) {
    serialEventRun();
  }
  sei();
  counter++;
}

void setup(void)
{

	int button_state;

	/* initialize the digital pins using arduino functions */













	/* Arduino has no API to change the INTO edge without re-init INT function name again" */
	/* attachInterrupt(digitalPinToInterrupt(button), ButtonISR, FALLING); */
	/* Use direct register configuration
	 * 	External Interrupt Control Register A
	 *	Bits		7 	6 	5	4	3		2		1		0
	 *	EICRA		-	-	-	-	ISC11	ISC10	ISC01	ISC00
	 *
	 *		ISC Bit Settings
	 *		ISCx1	ISCx0	DESCRIPTION
	 *			0		0	Low level of INTx generates an interrupt request
	 *			0		1	Any logic change on INTx generates an interrupt request
	 *			1		0	The falling edge of INTx generates an interrupt request
	 *			1		1	The rising edge of INTx generates an interrupt request
	 *
	 *	External Interrupt Mask Register
	 *	Bits		7 	6 	5	4	3	2	1		0
	 *	EIMSK		-	-	-	-	-	-	INT1	INT0
	 *
	 *	External Interrupt Flag Register
	 *	Bits		7 	6 	5	4	3	2	1		0
	 *  EIFR		-	-	-	-	-	-	INTF1	INTF0
	 */
	EICRA |= (1 << ISC01);    /* Trigger INT0 on falling edge */
	EIMSK |= (1 << INT0);     /* Enable external interrupt INT0 */


	Serial.begin(115200);
	Serial.println("My Shade Controller!");
}

int main(void)
{

	init();

	setup();

#if defined(USBCON)
	USBDevice.attach();
#endif

	StartOS(OSDEFAULTAPPMODE);		/* OSEE - OSEK init */

	return 0;
}

}	/* extern "C" */
