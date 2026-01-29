/** \file	debug.cpp
 *  \brief	This is the print serial utilities.
 *
 *  Created on: 2 Nov 2019
 *      Author:
 */

/* ERIKA Enterprise. */
#include "ee.h"

/* Arduino SDK. */
#include "Arduino.h"

extern "C" {

void serial_print(char const * msg) {
  cli();
  Serial.write(msg, strlen(msg));
  /*  if (serialEventRun) {
      serialEventRun();
  } */
  sei();
}

}	/* extern "C" */
