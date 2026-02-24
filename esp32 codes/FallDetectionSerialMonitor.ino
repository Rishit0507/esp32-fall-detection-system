#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"

/* ---------------- OLED ---------------- */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ---------------- PINS ---------------- */
#define BUZZER_PIN 26
#define BUTTON_PIN 27          // Cancel + triple click
#define PANIC_BUTTON 25        // NEW panic button

Adafruit_MPU6050 mpu;
MAX30105 particleSensor;

/* ---------------- STATES ---------------- */
enum SystemState {
  IDLE,
  IMPACT_DETECTED,
  FALL_CONFIRMED,
  EMERGENCY_ALERT
};

SystemState currentState = IDLE;

/* ---------------- TIMING ---------------- */
unsigned long immobileStart = 0;
unsigned long alertStart = 0;

/* ---------------- HEART MODE ---------------- */
bool heartMode = false;
unsigned long lastBeat = 0;
float bpm = 0;

/* ---------------- TRIPLE CLICK ---------------- */
int clickCount = 0;
unsigned long lastClickTime = 0;
const unsigned long clickTimeout = 1000;

/* ---------------- BUZZ ---------------- */
void buzz(unsigned int duration_ms) {
  unsigned long start = millis();
  while (millis() - start < duration_ms) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(500);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(500);
  }
}

/* ---------------- DISPLAY HELPERS ---------------- */
void centerText(String text, int textSize, int y) {
  display.setTextSize(textSize);
  display.setTextColor(WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - w) / 2;
  display.setCursor(x, y);
  display.println(text);
}

void showIdleScreen() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  centerText("MONITORING", 2, 20);
  display.display();
}

void showImpactScreen() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  centerText("IMPACT!", 2, 20);
  display.display();
}

void showFallScreen() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  centerText("FALL", 3, 5);
  centerText("Press to Cancel", 1, 50);
  display.display();
}

void showEmergencyScreen() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  centerText("EMERGENCY!", 2, 10);
  centerText("ALERT SENT", 2, 35);
  display.display();
}

void showHeartScreen(int bpmValue) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  centerText("HEART RATE", 1, 5);
  centerText(String(bpmValue) + " BPM", 3, 25);
  display.display();
}

/* ---------------- SETUP ---------------- */
void setup() {

  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PANIC_BUTTON, INPUT_PULLUP);   // NEW

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  mpu.begin();
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  particleSensor.begin(Wire, I2C_SPEED_STANDARD);
  particleSensor.setup(60, 2, 2, 100, 411, 4096);
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  showIdleScreen();
}

/* ---------------- HEART MODE FUNCTION ---------------- */
void runHeartMode() {

  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    bpm = 60 / (delta / 1000.0);
  }

  if (irValue > 5000) {
    showHeartScreen((int)bpm);
  } else {
    display.clearDisplay();
    centerText("PLACE FINGER", 2, 20);
    display.display();
  }

  delay(20);
}

/* ---------------- FALL MODE FUNCTION ---------------- */
void runFallMode() {

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float magnitude = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  bool cancelPressed = (digitalRead(BUTTON_PIN) == LOW);

  switch (currentState) {

    case IDLE:
      if (magnitude > 25) {
        currentState = IMPACT_DETECTED;
        immobileStart = 0;
        showImpactScreen();
      }
      break;

    case IMPACT_DETECTED:
      if (magnitude > 8 && magnitude < 12) {
        if (immobileStart == 0)
          immobileStart = millis();

        if (millis() - immobileStart > 2000) {
          currentState = FALL_CONFIRMED;
          alertStart = millis();
          showFallScreen();
        }
      } else {
        immobileStart = 0;
      }
      break;

    case FALL_CONFIRMED:
      buzz(200);
      delay(200);

      if (cancelPressed) {
        currentState = IDLE;
        showIdleScreen();
      }

      if (millis() - alertStart > 10000) {
        currentState = EMERGENCY_ALERT;
        showEmergencyScreen();
      }
      break;

    case EMERGENCY_ALERT:
      buzz(100);
      delay(100);

      if (cancelPressed) {
        currentState = IDLE;
        showIdleScreen();
      }
      break;
  }

  delay(50);
}

/* ---------------- LOOP ---------------- */
void loop() {

  /* ===== PANIC BUTTON (GLOBAL OVERRIDE) ===== */
  if (digitalRead(PANIC_BUTTON) == LOW && currentState != EMERGENCY_ALERT) {
    currentState = EMERGENCY_ALERT;
    showEmergencyScreen();
    delay(300);   // debounce
  }

  /* ===== Triple Click Detection (UNCHANGED) ===== */
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState == LOW && lastButtonState == HIGH) {
    clickCount++;
    lastClickTime = millis();
    delay(150);
  }

  lastButtonState = currentButtonState;

  if (clickCount > 0 && millis() - lastClickTime > clickTimeout) {
    if (clickCount == 3) {
      heartMode = !heartMode;
      if (!heartMode) {
        showIdleScreen();
      }
    }
    clickCount = 0;
  }

  /* ===== Mode Selection ===== */
  if (heartMode) {
    runHeartMode();
  } else {
    runFallMode();
  }
}
