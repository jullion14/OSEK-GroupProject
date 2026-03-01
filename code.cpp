/* ERIKA Enterprise. */
#include "ee.h"

/* Arduino SDK. */
#include "Arduino.h"
#include "LiquidCrystal.h"
#include "ServoTimer2.h"

/* Local Headers */
#include "hwpins.h"
#include "lcd.h"



extern "C" {

/* External Functions */
extern void serial_print(char const * msg);

extern class LiquidCrystal lcd;
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

	//Timing Btn
	pinMode(BTN_PLUS, INPUT_PULLUP);
	pinMode(BTN_MINUS, INPUT_PULLUP);

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
