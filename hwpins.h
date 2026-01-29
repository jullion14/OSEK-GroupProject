/*  \file	hwpins.h
 *  \brief	This is the header to define the pins and macros.
 */

#ifndef HWPINS_H_
#define HWPINS_H_

/* ===== SimulIDE wiring (Project.simu) ===== */
#define LED_West        (2)
#define LED_East        (3)

#define BUTTON          (4)     /* Button on D4 (PORTD) */

#define Servo_West      (5)
#define Servo_East      (6)

#define LDR_West        (A1)
#define LDR_East        (A0)

/* ===== LCD pins ===== */
#define RS				(8)		/* LCD RS Pin */
#define EN				(9) 	/* LCD EN Pin */
#define D4				(10)	/* LCD D4 Pin */
#define D5				(11)	/* LCD D5 Pin */
#define D6				(12)	/* LCD D6 Pin */
#define D7				(13)	/* LCD D7 Pin */

/* ===== Servo pulse macros (ServoTimer2) ===== */
#define Servo_West_0    (750)
#define Servo_West_90   (1500)
#define Servo_West_180	(2250)

#define Servo_East_0	(750)
#define Servo_East_90   (1500)
#define Servo_East_180	(2250)

#endif /* HWPINS_H_ */
