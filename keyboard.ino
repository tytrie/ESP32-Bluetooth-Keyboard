#include <BleKeyboard.h>
#define KEY_FN 0xF0
BleKeyboard bleKeyboard("ESP32_60Percent", "Custom Co.", 100);

// ========= MATRIX SIZE =========
const int numRows = 5;
const int numCols = 15;

// ========= PINS (SAFE FOR ESP32-WROOM-32D) =========
const int rowPins[numRows] = { 34, 12, 14, 27, 26 };
const int colPins[numCols] = { 25, 33, 32, 15, 2, 23, 4, 16, 17, 5, 18, 19, 21, 22, 13};

// ========= KEYMAP =========
const uint8_t keymap[numRows][numCols] = {
  { KEY_ESC, '1','2','3','4','5','6','7','8','9','0','-','=',KEY_BACKSPACE, '`' },
  { KEY_TAB, 'q','w','e','r','t','y','u','i','o','p','[',']','\\', KEY_PAGE_UP},
  { KEY_CAPS_LOCK, 'a','s','d','f','g','h','j','k','l',';', '\'', KEY_RETURN, KEY_DELETE, 0 },
  { KEY_LEFT_SHIFT, 'z','x','c','v','b','n','m',',','.','/', KEY_RIGHT_SHIFT, KEY_UP_ARROW, KEY_PAGE_DOWN, 0 },
  { KEY_LEFT_CTRL, KEY_LEFT_GUI, KEY_LEFT_ALT, ' ', ' ', ' ', KEY_RIGHT_ALT, 0, KEY_RIGHT_CTRL, KEY_FN, KEY_RIGHT_CTRL, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW, 0}
};

bool keyState[5][15] = {0};
unsigned long lastDebounceTime[5][15] = {0};
const unsigned long debounceDelay = 5; // 5ms debounce

bool fnActive = false;

const uint8_t fnLayer[numRows][numCols] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,KEY_DELETE,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0,KEY_UP_ARROW,0,0 },
  { 0,0,0,KEY_FN,0,0,0,0,0,0,KEY_LEFT_ARROW,KEY_DOWN_ARROW,KEY_RIGHT_ARROW,0,0 }
};


void setup() {
  Serial.begin(115200);
  Serial.println("Starting keyboard…");
  
  // Row pins as INPUT_PULLDOWN
  for (int r = 0; r < numRows; r++) {
    pinMode(rowPins[r], INPUT_PULLDOWN);
  }
  
  // Column pins as OUTPUT, initially LOW
  for (int c = 0; c < numCols; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], LOW);
  }
  
  bleKeyboard.begin();
}

void loop() {
  if (!bleKeyboard.isConnected()) return;
  
  for (int c = 0; c < numCols; c++) {
    // Activate column
    digitalWrite(colPins[c], HIGH);
    delayMicroseconds(10); // Allow signal to settle
    
    for (int r = 0; r < numRows; r++) {
      int reading = digitalRead(rowPins[r]);
      unsigned long currentTime = millis();
      
      // Check if enough time has passed for debouncing
      if ((currentTime - lastDebounceTime[r][c]) > debounceDelay) {
        
        // Key pressed
        if (reading == HIGH && !keyState[r][c]) {
          keyState[r][c] = true;
          lastDebounceTime[r][c] = currentTime;
          uint8_t k = keymap[r][c];

          if (k == KEY_FN) {
              fnActive = true;  // Hold FN
          } else {
              if (fnActive) k = fnLayer[r][c];
              if (k) bleKeyboard.press(k);
          }

          if (k) {
            bleKeyboard.press(k);
            Serial.print("Press: Row ");
            Serial.print(r);
            Serial.print(" Col ");
            Serial.println(c);
          }
        }
        
        // Key released
        if (reading == LOW && keyState[r][c]) {
          keyState[r][c] = false;
          lastDebounceTime[r][c] = currentTime;
          uint8_t k = keymap[r][c];

          if (k == KEY_FN) {
              fnActive = false;
          } else {
              if (fnActive) k = fnLayer[r][c];
              if (k) bleKeyboard.release(k);
          }


          if (k) {
            bleKeyboard.release(k);
            Serial.print("Release: Row ");
            Serial.print(r);
            Serial.print(" Col ");
            Serial.println(c);
          }
        }
      }
    }
    
    // Deactivate column
    digitalWrite(colPins[c], LOW);
    delayMicroseconds(10); // Prevent crosstalk
  }
}