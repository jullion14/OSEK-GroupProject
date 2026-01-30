///* ERIKA Enterprise. */
//#include "ee.h"
//
///* Arduino SDK. */
//#include "Arduino.h"
//
///* LCD + Servo Drivers */
//#include "LiquidCrystal.h"
//#include "ServoTimer2.h"
//
///* Local Headers */
//#include "hwpins.h"
//#include "lcd.h"
//
///* AVR registers for PCINT setup */
//#include <avr/io.h>
//
//extern "C" {
//
///* ===== Forward declarations (implemented in task.cpp) ===== */
//extern class LiquidCrystal lcd;
//extern class ServoTimer2 servoWest;
//extern class ServoTimer2 servoEast;
//
///* Idle hook (required by OIL) */
//void idle_hook(void) {
//  /* keep it empty/minimal */
//}
//
///* Optional StartupHook (enabled in OIL). Good place for init too. */
//void StartupHook(void) {
//  /* You can move init here if your prof prefers hooks; setup() is also fine. */
//}
//
//void setup(void)
//{
//  /* ===== IO init ===== */
//  pinMode(LED_East, OUTPUT);
//  digitalWrite(LED_East, LOW);
//
//  pinMode(LED_West, OUTPUT);
//  digitalWrite(LED_West, LOW);
//
//  pinMode(LDR_East, INPUT);
//  pinMode(LDR_West, INPUT);
//
//  pinMode(BUTTON, INPUT_PULLUP);   /* typical SimulIDE pushbutton to GND */
//
//  /* Servo outputs */
//  pinMode(Servo_West, OUTPUT);
//  pinMode(Servo_East, OUTPUT);
//
//  servoWest.attach(Servo_West);
//  servoWest.write(Servo_West_0);   /* start contracted */
//
//  servoEast.attach(Servo_East);
//  servoEast.write(Servo_East_0);   /* start contracted */
//
//  /* LCD init */
//  lcd.begin(LCD_COL, LCD_ROW);
//  lcd.createChar(CUSTOMCHAR1, mapChar1);
//  lcd.clear();
//
//  Serial.begin(115200);
//  Serial.println("Shade Controller (OSEK)");
//
//  /* ===== Enable Pin Change Interrupt for D4 (PD4 = PCINT20) =====
//     - PD4 is in Port D group => PCINT2 group (PCIE2 / PCMSK2)
//  */
//  PCICR |= (1 << PCIE2);          /* enable PCINT[23:16] */
//  PCMSK2 |= (1 << PCINT20);       /* enable PD4 */
//  PCIFR |= (1 << PCIF2);          /* clear any pending flag */
//}
//
//int main(void)
//{
//  init();
//  setup();
//
//#if defined(USBCON)
//  USBDevice.attach();
//#endif
//
//  StartOS(OSDEFAULTAPPMODE);
//  return 0;
//}
//
//} /* extern "C" */

//JX Edits

/* ERIKA Enterprise. */
#include "ee.h"

/* Arduino SDK. */
#include "Arduino.h"
#include "LiquidCrystal.h"						// LCD - Add header
#include "ServoTimer2.h"

/* Local Headers */
#include "hwpins.h"
#include "lcd.h"



extern "C" {

/* External Functions */
extern void serial_print(char const * msg);

extern class LiquidCrystal lcd;  	// LCD - Add Class object
extern ServoTimer2 servoWest;
extern ServoTimer2 servoEast;

/* Local Variables */
boolean   volatile stk_wrong = false;
OsEE_addr volatile old_sp;
uint32_t volatile idle_cnt;

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


void idle_hook(void)
{
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
}

void setup(void)
{
	// set up the LCD's number of columns and rows:
	lcd.begin(LCD_COL, LCD_ROW);
	lcd.createChar(CUSTOMCHAR1, mapChar1);		//create custom character 1



	Serial.begin(9600);
	Serial.println("Hello OSEK-Arduino!");


	//LCD.
	lcd.clear();

	//LDR
	pinMode(LDR_West, INPUT);
	pinMode(LDR_East, INPUT);

	//LED
	pinMode(LED_West, OUTPUT);
	pinMode(LED_East, OUTPUT);

	digitalWrite(LED_West, LOW);
	digitalWrite(LED_East, LOW);

	//Servo
	servoWest.attach(Servo_West);
	servoEast.attach(Servo_East);
	servoWest.write(Servo_West_0);
	servoEast.write(Servo_East_0);

}

int main(void)
{

	init();

	setup();						/* User defined pin declaration */

#if defined(USBCON)
	USBDevice.attach();
#endif

	StartOS(OSDEFAULTAPPMODE);		/* OSEE - OSEK init */

	return 0;

}

}	/* extern "C" */
