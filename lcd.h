/*
 *  \file	lcd.h
 *  \brief	This is the header to define the lcd macros.
 *
 *  Created on: 15 Jul 2021
 *      Author:
 */

#ifndef LCD_H_
#define LCD_H_

//User configuration for LCD row and column
#define LCD_ROW			(4)
#define LCD_COL			(20)

// See Link
// https://www.quinapalus.com/hd44780udg.html
// or
// https://omerk.github.io/lcdchargen/

/* User can define up to 8 custom character, tag from 0 to 7 */
#define CUSTOMCHAR1		(1)		//Custom Character tag 1, only 0x00 to 0x07 in CGRAM
static uint8_t mapChar1[8] = {	//bitmap for special display - Heart
  B00000,
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

#endif /* LCD_H_ */
