#include "ee.h"
#include "Arduino.h"
#include "LiquidCrystal.h"
#include "ServoTimer2.h"

#include "hwpins.h"
#include "lcd.h"

extern "C" {

/* LCD object definition*/
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

/* Task declarations */
DeclareTask(DetectLightTask);
DeclareTask(DisplayTask);


ServoTimer2 servoWest;
ServoTimer2 servoEast;

/* ----------------- LDR + Lux constants ----------------- */
#define NIGHT_THRESHOLD     (200.0f)

#define MAX_ADC_READING     (1023.0f)
#define ADC_REF_VOLTAGE     (5.0f)
#define REF_RESISTANCE      (5000.0f)
#define LUX_CALC_SCALAR     (889985.88f)
#define LUX_CALC_EXPONENT   (-1.16552f)
#define SERVO_LUX_THRESHOLD (500.0f)


/* ----------------- LCD Global Variables----------------- */
volatile float luxWest = 0.0f;
volatile float luxEast = 0.0f;

volatile bool shadeOn = false;  // placeholder
volatile bool lightOn = false;  // we’ll link this to LEDs below

volatile uint8_t hh = 12, mm = 0, ss = 0;

/* ----------------- Helpers ----------------- */
static void lcdPrintPadded(uint8_t col, uint8_t row, const char *s)
{
  lcd.setCursor(col, row);

  uint8_t i = 0;
  while (s[i] != '\0' && i < LCD_COL) {
    lcd.print(s[i]);
    i++;
  }
  while (i < LCD_COL) {
    lcd.print(' ');
    i++;
  }
}

static void tickClock_500ms(void)
{
  static uint8_t half = 0;
  half ^= 1;
  if (half == 0) {             // every 1s
    ss++;
    if (ss >= 60) { ss = 0; mm++; }
    if (mm >= 60) { mm = 0; hh++; }
    if (hh >= 24) { hh = 0; }
  }
}

static float adcToLux(int raw)
{
  float resistorVoltage = (float)raw / MAX_ADC_READING * ADC_REF_VOLTAGE;

  if (resistorVoltage < 0.001f) resistorVoltage = 0.001f;
  if (resistorVoltage > (ADC_REF_VOLTAGE - 0.001f)) resistorVoltage = ADC_REF_VOLTAGE - 0.001f;

  float ldrVoltage = ADC_REF_VOLTAGE - resistorVoltage;
  float ldrResistance = (ldrVoltage / resistorVoltage) * REF_RESISTANCE;

  return (float)(LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT));
}

/* ----------------- Task 1: Read LDRs + control LEDs ----------------- */
TASK(DetectLightTask)
{
  int rawW = analogRead(LDR_West);
  int rawE = analogRead(LDR_East);

  luxWest = adcToLux(rawW);
  luxEast = adcToLux(rawE);

  float luxAvg = (luxWest + luxEast) * 0.5f;

  //LIGHT status + LEDs
  lightOn = (luxAvg < NIGHT_THRESHOLD);

  digitalWrite(LED_West, lightOn ? HIGH : LOW);
  digitalWrite(LED_East, lightOn ? HIGH : LOW);

  //SHADE status + Servos
  shadeOn = (luxAvg >= SERVO_LUX_THRESHOLD);

  servoWest.write(shadeOn ? Servo_West_180 : Servo_West_0);
  servoEast.write(shadeOn ? Servo_East_180 : Servo_East_0);

}

/* ----------------- Task 2: Update LCD UI ----------------- */
TASK(DisplayTask)
{
  tickClock_500ms();

  char line[32];

  // L1: LUX: xxxW xxxA xxxE
  float luxAvg = (luxWest + luxEast) * 0.5f;

  snprintf(line, sizeof(line), "LUX:%3dW %3dA %3dE",
           (int)luxWest, (int)luxAvg, (int)luxEast);
  lcdPrintPadded(0, 0, line);

  // L2: Shade:
  snprintf(line, sizeof(line), "Shade: %s", shadeOn ? "ON" : "OFF");
  lcdPrintPadded(0, 1, line);

  // L3: Light:
  snprintf(line, sizeof(line), "Light: %s", lightOn ? "ON" : "OFF");
  lcdPrintPadded(0, 2, line);

  // L4: Clock:
  snprintf(line, sizeof(line), "Clock: %02u:%02u:%02u", hh, mm, ss);
  lcdPrintPadded(0, 3, line);

  TerminateTask();
}

} /* extern "C" */

