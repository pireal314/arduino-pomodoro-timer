// key press - https://forum.arduino.cc/t/button-pressed-more-than-2-seconds-do-somethingelse/136822/7
// oled display - https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

// Required libraries:
// - Adafuit SSD1306 (plus dependencies)

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define SHORT_MODE 0 // Short timer duration (25 minutes)
#define LONG_MODE 1  // Long timer duration (50 minutes)

unsigned long keyPrevMillis = 0;
const unsigned long keySampleIntervalMs = 25;
byte longKeyPressCountMax = 80;    // 80 * 25 = 2000 ms
byte mediumKeyPressCountMin = 20;    // 20 * 25 = 500 ms
byte KeyPressCount = 0;

byte prevKeyState = HIGH;         // button is active low
const byte keyPin = 2;            // button is connected to pin 2 and GND

int timerMode = LONG_MODE;
bool modeChanged = 0;
bool breakMode = 0;
int timerMinutes = 50;
int timerSeconds = 00;
unsigned long timerPrevMillis = 0;
String displayString;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// called when button is kept pressed for less than .5 seconds
void shortKeyPress() {
    Serial.println("short key press");
  
    if (breakMode) {
      if (timerMode == LONG_MODE) {
        timerMinutes = 50;
      } else {
        timerMinutes = 25;
      }
    } else {
      if (timerMode == LONG_MODE) {
        timerMinutes = 10; 
      } else {
        timerMinutes = 5;
      }
    }

    timerSeconds = 0;
    breakMode = !breakMode;
}

// called when button is kept pressed for 2 seconds or more
void longKeyPress() {
  Serial.println("long key press");

  if (!modeChanged) {
    if (timerMode == LONG_MODE) {
      timerMode = SHORT_MODE;
      timerMinutes = 25;
    } else {
      timerMode = LONG_MODE;
      timerMinutes = 50;
    }
 
    timerSeconds = 0;
    modeChanged = 1;
    breakMode = 0;
  }
}

void keyPress() {
    Serial.println("key press");
  
    KeyPressCount = 0;
}

void keyRelease() {
  Serial.println("key release");
  
  if (KeyPressCount < mediumKeyPressCountMin) {
    shortKeyPress();
  }

  modeChanged = 0;
}

void updateTimer() {
  if (timerMinutes > 0 || timerSeconds > 0) {
    if (timerSeconds > 0) {
      timerSeconds--;
    }
    else {
      timerSeconds = 59;
  
      if (timerMinutes > 0)
        timerMinutes--;
    }

    timerPrevMillis = millis();
  }
}

void updateDisplayString() {
  String minutes = String(timerMinutes);
  String seconds = String(timerSeconds);
  
  if (timerMinutes < 10) {
    minutes = "0" + minutes;
  }

  if (timerSeconds < 10) {
    seconds = "0" + seconds;
  }

  displayString = minutes + ":" + seconds;
}


// ----------------------------------------------------------
// Arduino hook functions
// ----------------------------------------------------------

void setup() {
    Serial.begin(9600);
    pinMode(keyPin, INPUT_PULLUP);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
    }

    display.setTextSize(3);
    display.setTextColor(WHITE);
}


void loop() {
    // monitor button presses
    if (millis() - keyPrevMillis >= keySampleIntervalMs) {
        keyPrevMillis = millis();
        
        byte currKeyState = digitalRead(keyPin);
        
        if ((prevKeyState == HIGH) && (currKeyState == LOW)) {
          keyPress();
        }
        else if ((prevKeyState == LOW) && (currKeyState == HIGH)) {
          keyRelease();
        }
        else if (currKeyState == LOW) {
          KeyPressCount++;

          if (KeyPressCount >= longKeyPressCountMax) {
            longKeyPress();
          }
        }
        
        prevKeyState = currKeyState;
    }
    
    // update display
    if (millis() - timerPrevMillis >= 1000) {
      updateTimer();
      updateDisplayString();
      
      display.clearDisplay();
      display.setCursor(21, 6);
  
      display.println(displayString);
      display.display();
    }
}
