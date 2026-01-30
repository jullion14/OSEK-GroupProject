///* ERIKA Enterprise. */
//#include "ee.h"
//
///* Arduino SDK. */
//#include "Arduino.h"
//
//#include <math.h>
//
///* Drivers */
//#include "LiquidCrystal.h"
//#include "ServoTimer2.h"
//
///* Local Headers */
//#include "hwpins.h"
//#include "lcd.h"
//
//extern "C" {
//
///* External serial print helper (debug.cpp) */
//extern void serial_print(char const * msg);
//
///* ===== Task declarations (match conf.oil names) ===== */
//DeclareTask(DetectTask);
//DeclareTask(DisplayTask);
//DeclareTask(ButtonTask);
//
///* ===== Resource (match conf.oil) ===== */
//DeclareResource(SharedRes);
//
///* ===== ISR2 declaration (match conf.oil ISR name) ===== */
//ISR2(ButtonISR);
//
///* ===== Global driver objects (as shown in project appendices) ===== */
//LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);  /* 4-bit mode */
//ServoTimer2 servoWest;
//ServoTimer2 servoEast;
//
///* ===== Lux conversion constants (from project guide) ===== */
//#define NIGHT_THRESHOLD   (200.0f)   /* <=200 lux -> night */
//#define DAY_THRESHOLD     (500.0f)   /* >=500 lux -> day */
//
//#define MAX_ADC_READING   (1023.0f)
//#define ADC_REF_VOLTAGE   (5.0f)
//#define REF_RESISTANCE    (5000.0f)
//#define LUX_CALC_SCALAR   (889985.88f)
//#define LUX_CALC_EXPONENT (-1.16552f)
//
///* ===== Servo positions ===== */
//#define SHADE_CONTRACT_W  (Servo_West_0)
//#define SHADE_EXPAND_W    (Servo_West_180)
//
//#define SHADE_CONTRACT_E  (Servo_East_0)
//#define SHADE_EXPAND_E    (Servo_East_180)
//
///* ===== Shared state (protected by SharedRes) ===== */
//static float gLuxWest = 0.0f;
//static float gLuxEast = 0.0f;
//static float gLuxAvg  = 0.0f;
//
//typedef enum {
//  MODE_TWILIGHT = 0,
//  MODE_NIGHT,
//  MODE_DAY
//} Mode_t;
//
//static Mode_t gMode = MODE_TWILIGHT;
//static uint8_t gPage = 0;
//
///* Local helper: convert ADC to Lux */
//static float adc_to_lux(int adc)
//{
//  float resistorVoltage = ((float)adc / MAX_ADC_READING) * ADC_REF_VOLTAGE;
//
//  /* avoid division by zero / invalid */
//  if (resistorVoltage < 0.001f) {
//    resistorVoltage = 0.001f;
//  }
//  if (resistorVoltage > (ADC_REF_VOLTAGE - 0.001f)) {
//    resistorVoltage = ADC_REF_VOLTAGE - 0.001f;
//  }
//
//  float ldrVoltage = ADC_REF_VOLTAGE - resistorVoltage;
//  float ldrResistance = (ldrVoltage / resistorVoltage) * REF_RESISTANCE;
//  float lux = LUX_CALC_SCALAR * powf(ldrResistance, LUX_CALC_EXPONENT);
//
//  if (lux < 0.0f) lux = 0.0f;
//  return lux;
//}
//
///* ===== ISR2: trigger ButtonTask ===== */
//ISR2(ButtonISR)
//{
//  ActivateTask(ButtonTask);
//}
//
///* ===== ButtonTask: toggle LCD page (simple, safe) ===== */
//TASK(ButtonTask)
//{
//  /* simple edge-ish filter: only act if currently pressed (LOW with pullup) */
//  if (digitalRead(BUTTON) == LOW) {
//    GetResource(SharedRes);
//    gPage = (uint8_t)((gPage + 1) % 2);
//    ReleaseResource(SharedRes);
//  }
//  TerminateTask();
//}
//
///* ===== DetectTask: read LDRs, compute lux, control LEDs + shade ===== */
//TASK(DetectTask)
//{
//  int adcW = analogRead(LDR_West);
//  int adcE = analogRead(LDR_East);
//
//  float luxW = adc_to_lux(adcW);
//  float luxE = adc_to_lux(adcE);
//  float avg  = (luxW + luxE) * 0.5f;
//
//  Mode_t mode;
//  if (avg <= NIGHT_THRESHOLD) {
//    mode = MODE_NIGHT;
//  } else if (avg >= DAY_THRESHOLD) {
//    mode = MODE_DAY;
//  } else {
//    mode = MODE_TWILIGHT;
//  }
//
//  /* Actuators */
//  if (mode == MODE_NIGHT) {
//    digitalWrite(LED_West, HIGH);
//    digitalWrite(LED_East, HIGH);
//
//    /* shade contracted */
//    servoWest.write(SHADE_CONTRACT_W);
//    servoEast.write(SHADE_CONTRACT_E);
//  } else if (mode == MODE_DAY) {
//    digitalWrite(LED_West, LOW);
//    digitalWrite(LED_East, LOW);
//
//    /* shade expanded */
//    servoWest.write(SHADE_EXPAND_W);
//    servoEast.write(SHADE_EXPAND_E);
//  } else {
//    digitalWrite(LED_West, LOW);
//    digitalWrite(LED_East, LOW);
//
//    /* shade contracted */
//    servoWest.write(SHADE_CONTRACT_W);
//    servoEast.write(SHADE_CONTRACT_E);
//  }
//
//  /* publish shared state */
//  GetResource(SharedRes);
//  gLuxWest = luxW;
//  gLuxEast = luxE;
//  gLuxAvg  = avg;
//  gMode    = mode;
//  ReleaseResource(SharedRes);
//
//  TerminateTask();
//}
//
///* ===== DisplayTask: update LCD ===== */
//TASK(DisplayTask)
//{
//  float luxW, luxE, avg;
//  Mode_t mode;
//  uint8_t page;
//
//  GetResource(SharedRes);
//  luxW = gLuxWest;
//  luxE = gLuxEast;
//  avg  = gLuxAvg;
//  mode = gMode;
//  page = gPage;
//  ReleaseResource(SharedRes);
//
//  lcd.setCursor(0, 0);
//  lcd.print("W:");
//  lcd.print((int)luxW);
//  lcd.print("lx E:");
//  lcd.print((int)luxE);
//  lcd.print("lx     "); /* clear tail */
//
//  lcd.setCursor(0, 1);
//  lcd.print("AVG:");
//  lcd.print((int)avg);
//  lcd.print("lx ");
//
//  if (mode == MODE_NIGHT) {
//    lcd.print("NIGHT ");
//  } else if (mode == MODE_DAY) {
//    lcd.print("DAY   ");
//  } else {
//    lcd.print("MID   ");
//  }
//
//  lcd.setCursor(0, 2);
//  if (page == 0) {
//    lcd.print("Lights:");
//    lcd.print((mode == MODE_NIGHT) ? "ON " : "OFF");
//    lcd.print(" Shade:");
//    lcd.print((mode == MODE_DAY) ? "EXP" : "CON");
//    lcd.print("   ");
//  } else {
//    lcd.print("Btn toggles page ");
//    lcd.write(CUSTOMCHAR1);
//    lcd.print("     ");
//  }
//
//  lcd.setCursor(0, 3);
//  lcd.print("DTask200ms LCD500 ");
//  lcd.print("      ");
//
//  TerminateTask();
//}
//
//} /* extern "C" */

//JX Edits

#include "ee.h"
#include "Arduino.h"
#include "LiquidCrystal.h"
#include "ServoTimer2.h"

#include "hwpins.h"
#include "lcd.h"

extern "C" {

/* LCD object defined once here */
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

/* Task declarations must match conf.oil names */
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

  // Avoid divide-by-zero if sensor saturates
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

  // L1: LUX: xxxW xxxA xxxE  (you only have W/E now, so we’ll show A as average)
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

