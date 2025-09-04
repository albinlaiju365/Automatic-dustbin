#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <LedControl.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Servo pins
Servo servoDry;
Servo servoWet;
#define SERVO_DRY_PIN 9
#define SERVO_WET_PIN 10

// MAX7219 setup (2 displays, each with separate CS pin)
LedControl dryDisplay = LedControl(12, 11, 7, 1); // Dry bin
LedControl wetDisplay = LedControl(12, 11, 8, 1); // Wet bin

// Stats
int people = 0;
int dry = 0;
int wet = 0;

void setup() {
  Serial.begin(9600);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Servo init
  servoDry.attach(SERVO_DRY_PIN);
  servoWet.attach(SERVO_WET_PIN);
  servoDry.write(0); // Closed
  servoWet.write(0); // Closed

  // MAX7219 init
  dryDisplay.shutdown(0, false);
  dryDisplay.setIntensity(0, 8);
  dryDisplay.clearDisplay(0);

  wetDisplay.shutdown(0, false);
  wetDisplay.setIntensity(0, 8);
  wetDisplay.clearDisplay(0);

  updateOLED();
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == '1') {   // Dry bin
      people++;
      dry++;
      openServo(servoDry, dryDisplay);
      updateOLED();
    }
    else if (cmd == '2') {  // Wet bin
      people++;
      wet++;
      openServo(servoWet, wetDisplay);
      updateOLED();
    }
    // ignore '0'
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Smart Dustbin Stats");

  display.setCursor(0, 16);
  display.print("People: "); display.println(people);

  display.setCursor(0, 32);
  display.print("Dry: "); display.println(dry);

  display.setCursor(0, 48);
  display.print("Wet: "); display.println(wet);

  display.display();
}

// Servo + light control
void openServo(Servo &servo, LedControl &lc) {
  turnOn(lc);          // Light ON when opening

  servo.write(90);     // Open
  delay(2000);         // Stay open 2 sec

  servo.write(0);      // Close
  delay(500);          // Give time to close

  turnOff(lc);         // Light OFF after closing
}

// Turn ON dot display (all LEDs)
void turnOn(LedControl &lc) {
  for (int row = 0; row < 8; row++) {
    lc.setRow(0, row, B11111111);
  }
}

// Turn OFF dot display
void turnOff(LedControl &lc) {
  lc.clearDisplay(0);
}
