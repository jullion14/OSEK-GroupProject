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

/* Returns true if time is within 18:30:00 .. next day 07:30:00 */
static bool isStreetlightClockWindow(uint8_t h, uint8_t m, uint8_t s)
{
  (void)s; // seconds not needed for the window logic

  const uint16_t start = 18u * 60u + 30u; // 18:30
  const uint16_t end   =  7u * 60u + 30u; // 07:30
  const uint16_t cur   = (uint16_t)h * 60u + (uint16_t)m;

  // Window crosses midnight: ON if cur >= start OR cur < end
  return (cur >= start) || (cur < end);
}

static void adjustTimeByMinutes(int deltaMin)
{
  // Make a local copy first (atomic-ish)
  uint8_t h = hh, m = mm, s = ss;

  int total = (int)h * 60 + (int)m;
  total += deltaMin;

  // wrap around 0..1439
  while (total < 0) total += 1440;
  while (total >= 1440) total -= 1440;

  // write back
  hh = (uint8_t)(total / 60);
  mm = (uint8_t)(total % 60);
  ss = s; // keep seconds unchanged
}

// Simple debounced "press event" detector for INPUT_PULLUP buttons
static bool buttonPressedEvent(uint8_t pin)
{
  // Debounce state per pin (supports two pins)
  typedef struct {
    uint8_t lastStable;
    uint8_t lastRead;
    uint32_t lastChangeMs;
  } BtnState;

  static BtnState stPlus  = { HIGH, HIGH, 0 };
  static BtnState stMinus = { HIGH, HIGH, 0 };

  BtnState *st = (pin == BTN_PLUS) ? &stPlus : &stMinus;

  uint8_t r = (uint8_t)digitalRead(pin);

  // track changes for debounce
  if (r != st->lastRead) {
    st->lastRead = r;
    st->lastChangeMs = millis();
  }

  // if stable for 40ms, accept as new stable
  if ((millis() - st->lastChangeMs) >= 40) {
    if (st->lastStable != st->lastRead) {
      st->lastStable = st->lastRead;

      // We only fire on the transition to PRESSED (LOW)
      if (st->lastStable == LOW) return true;
    }
  }

  return false;
}

/* ----------------- Task 1: Read LDRs + control LEDs ----------------- */
TASK(DetectLightTask)
{
  int rawW = analogRead(LDR_West);
  int rawE = analogRead(LDR_East);

  // Protect shared clock and lux variables during updates
  GetResource(SharedData);

  if (buttonPressedEvent(BTN_PLUS)) {
	  adjustTimeByMinutes(+30);
	  Serial.println("Plus pressed");
  }
  if (buttonPressedEvent(BTN_MINUS)) {
	  adjustTimeByMinutes(-30);
	  Serial.println("Minus pressed");
  }

  luxWest = adcToLux(rawW);
  luxEast = adcToLux(rawE);

  float luxAvg = (luxWest + luxEast) * 0.5f;

  // -------- LIGHT status + LEDs (Clock override) --------
  const bool forceOnByClock = isStreetlightClockWindow(hh, mm, ss);

  // Average LUX <= 200 => Switch On, >200 => Switch Off
  const bool onByLux = (luxAvg <= NIGHT_THRESHOLD);

  lightOn = forceOnByClock || onByLux;

  digitalWrite(LED_West, lightOn ? HIGH : LOW);
  digitalWrite(LED_East, lightOn ? HIGH : LOW);

  //SHADE status + Servos
  shadeOn = (luxAvg >= SERVO_LUX_THRESHOLD);

  servoWest.write(shadeOn ? Servo_West_180 : Servo_West_0);
  servoEast.write(shadeOn ? Servo_East_180 : Servo_East_0);

  ReleaseResource(SharedData);
  TerminateTask();

}

/* ----------------- Task 2: Update LCD UI ----------------- */
TASK(DisplayTask)
{
  // Protect variables while reading them for the UI
  GetResource(SharedData);
  tickClock_500ms();

  char line[32];

  // L1: LUX: xxxW xxxA xxxE
  float luxAvg = (luxWest + luxEast) * 0.5f;

  // --- Button handling: +/- 30 minutes on press ---

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

  ReleaseResource(SharedData);
  TerminateTask();
}

} /* extern "C" */

