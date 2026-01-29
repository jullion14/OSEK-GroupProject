/* ERIKA Enterprise. */
#include "ee.h"

/* Arduino SDK. */
#include "Arduino.h"

/* LCD + Servo Drivers */
#include "LiquidCrystal.h"
#include "ServoTimer2.h"

/* Local Headers */
#include "hwpins.h"
#include "lcd.h"

/* AVR registers for PCINT setup */
#include <avr/io.h>

extern "C" {

/* ===== Forward declarations (implemented in task.cpp) ===== */
extern class LiquidCrystal lcd;
extern class ServoTimer2 servoWest;
extern class ServoTimer2 servoEast;

/* Idle hook (required by OIL) */
void idle_hook(void) {
  /* keep it empty/minimal */
}

/* Optional StartupHook (enabled in OIL). Good place for init too. */
void StartupHook(void) {
  /* You can move init here if your prof prefers hooks; setup() is also fine. */
}

void setup(void)
{
  /* ===== IO init ===== */
  pinMode(LED_East, OUTPUT);
  digitalWrite(LED_East, LOW);

  pinMode(LED_West, OUTPUT);
  digitalWrite(LED_West, LOW);

  pinMode(LDR_East, INPUT);
  pinMode(LDR_West, INPUT);

  pinMode(BUTTON, INPUT_PULLUP);   /* typical SimulIDE pushbutton to GND */

  /* Servo outputs */
  pinMode(Servo_West, OUTPUT);
  pinMode(Servo_East, OUTPUT);

  servoWest.attach(Servo_West);
  servoWest.write(Servo_West_0);   /* start contracted */

  servoEast.attach(Servo_East);
  servoEast.write(Servo_East_0);   /* start contracted */

  /* LCD init */
  lcd.begin(LCD_COL, LCD_ROW);
  lcd.createChar(CUSTOMCHAR1, mapChar1);
  lcd.clear();

  Serial.begin(115200);
  Serial.println("Shade Controller (OSEK)");

  /* ===== Enable Pin Change Interrupt for D4 (PD4 = PCINT20) =====
     - PD4 is in Port D group => PCINT2 group (PCIE2 / PCMSK2)
  */
  PCICR |= (1 << PCIE2);          /* enable PCINT[23:16] */
  PCMSK2 |= (1 << PCINT20);       /* enable PD4 */
  PCIFR |= (1 << PCIF2);          /* clear any pending flag */
}

int main(void)
{
  init();
  setup();

#if defined(USBCON)
  USBDevice.attach();
#endif

  StartOS(OSDEFAULTAPPMODE);
  return 0;
}

} /* extern "C" */
