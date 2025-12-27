#include <Arduino.h>
#include <LiquidCrystal.h>
#include <BluetoothSerial.h>

// ===== LCD PINS =====
#define RS 23
#define E  22
#define D4 21
#define D5 19
#define D6 18
#define D7 5

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

// ===== LDR =====
#define LDR_PIN 34
#define THRESHOLD 2500

// ===== TOGGLE BUTTON =====
#define BUTTON_PIN 15
#define HOLD_TIME 2000  // 2 seconds to toggle

// ===== LED FOR TRANSMISSION =====
#define LED_TX_PIN 2  // Built-in LED or external LED

// ===== BLUETOOTH =====
BluetoothSerial SerialBT;
String btDataToSend = "";
bool btConnected = false;

// ===== MODE =====
enum Mode { RECEIVE_MODE, SEND_MODE };
Mode currentMode = RECEIVE_MODE;

// Bluetooth callbacks
void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if(event == ESP_SPP_SRV_OPEN_EVT) {
    btConnected = true;
    lcd.clear();
    lcd.print("BT Connected!");
    lcd.setCursor(0,1);
    if(currentMode == SEND_MODE) {
      lcd.print("Type to send");
    } else {
      lcd.print("Mode: RECEIVE");
    }
  } else if(event == ESP_SPP_CLOSE_EVT) {
    btConnected = false;
    lcd.clear();
    lcd.print("BT Disconnected");
    lcd.setCursor(0,1);
    lcd.print("Waiting...");
  }
}

// ===== Morse timing (Slower for better LDR detection) =====
#define DOT_DURATION 150
#define DASH_DURATION 450
#define LETTER_PAUSE 450
#define WORD_PAUSE 1050
#define TIMEOUT_PAUSE 2500

// Buffers
String morseBuffer = "";
String textBuffer = "";

// Morse mapping
struct MorseMap {
  const char* code;
  char letter;
} morseTable[] = {
  {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
  {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
  {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
  {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
  {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
  {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
  {"-.--", 'Y'}, {"--..", 'Z'},
  {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
  {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'},
  {"---..", '8'}, {"----.", '9'},
  // Symbols
  {".-..-.", '"'}, {"..--.-", '_'}, {".-.-.-", '.'}, {"--..--", ','},
  {"..--..", '?'}, {"-.-.--", '!'}, {"-..-.", '/'}, {"-....-", '-'},
  {".-.-.", '+'}, {"-...-", '='}, {"-.--.", '('}, {"-.--.-", ')'},
  {".-...", '&'}, {"---...", ':'}, {"-.-.-.", ';'}, {"...-..-", '$'},
  {".--.-.", '@'}, {"...-.-", '~'}, {".--..", '*'}
};

// Encode letter to Morse
String encodeMorse(char c) {
  c = toupper(c);
  if(c == ' ') return "/"; // Word separator
  for(int i = 0; i < sizeof(morseTable)/sizeof(MorseMap); i++) {
    if(c == morseTable[i].letter) return String(morseTable[i].code);
  }
  return "";
}

// Decode Morse to letter
char decodeMorse(String code) {
  for(int i = 0; i < sizeof(morseTable)/sizeof(MorseMap); i++) {
    if(code.equals(morseTable[i].code)) return morseTable[i].letter;
  }
  return '?';
}

// Scroll long text on 16x2 LCD - News ticker style
void scrollText(String text) {
  int totalLength = text.length();
  if(totalLength <= 16) {
    // Short message - fits on one line
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Received Msg:");
    lcd.setCursor(0,1);
    lcd.print(text);
  } else {
    // Long message - show initial preview
    lcd.clear();
    lcd.print("Received Msg:");
    lcd.setCursor(0,1);
    lcd.print(text.substring(0, 16));
    delay(1500);
  }
}

// Transmit a single character via LED as Morse code
void transmitChar(char c) {
  String morse = encodeMorse(c);
  if(morse == "/") {
    // Word separator
    delay(WORD_PAUSE);
    return;
  }
  
  for(int i = 0; i < morse.length(); i++) {
    if(morse[i] == '.') {
      digitalWrite(LED_TX_PIN, LOW);   // LOW = ON
      delay(DOT_DURATION);
      digitalWrite(LED_TX_PIN, HIGH);  // HIGH = OFF
      delay(DOT_DURATION);
    } else if(morse[i] == '-') {
      digitalWrite(LED_TX_PIN, LOW);   // LOW = ON
      delay(DASH_DURATION);
      digitalWrite(LED_TX_PIN, HIGH);  // HIGH = OFF
      delay(DOT_DURATION);
    }
  }
  delay(LETTER_PAUSE - DOT_DURATION); // Letter gap
}

// Transmit entire message via LED
void transmitMessage(String message) {
  lcd.clear();
  lcd.print("Transmitting...");
  lcd.setCursor(0,1);
  
  for(int i = 0; i < message.length(); i++) {
    // Show progress on LCD (up to 16 chars)
    if(i < 16) {
      lcd.print(message[i]);
    }
    transmitChar(message[i]);
  }
  
  // Ensure LED is OFF after transmission
  digitalWrite(LED_TX_PIN, HIGH);  // HIGH = OFF
  
  lcd.clear();
  lcd.print("TX Complete!");
  lcd.setCursor(0,1);
  lcd.print("Sent: ");
  lcd.print(message.substring(0, 10));
  delay(2000);
}

void setup() {
  // Initialize LCD
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("Li-Fi System");
  lcd.setCursor(0,1);
  lcd.print("Initializing...");
  
  // Initialize pins
  pinMode(LDR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_TX_PIN, OUTPUT);
  digitalWrite(LED_TX_PIN, HIGH);  // HIGH = OFF for built-in LED
  
  // Initialize Bluetooth with callback
  SerialBT.register_callback(btCallback);
  SerialBT.begin("ESP32_LiFi"); // Bluetooth device name
  delay(1000);
  
  lcd.clear();
  lcd.print("BT: ESP32_LiFi");
  lcd.setCursor(0,1);
  lcd.print("Pair & Connect!");
  delay(2000);
  
  lcd.clear();
  lcd.print("Mode: RECEIVE");
  lcd.setCursor(0,1);
  lcd.print("Waiting...");
  delay(1000);
}

void loop() {
  static unsigned long buttonPressStart = 0;
  static bool buttonPressed = false;
  static bool modeJustChanged = false;
  digitalWrite(LED_TX_PIN, HIGH);  // Keep LED OFF (HIGH = OFF)
  // ===== BUTTON HANDLING (Toggle Mode) =====
  int buttonState = digitalRead(BUTTON_PIN);
  
  if(buttonState == LOW && !buttonPressed) {
    // Button just pressed
    buttonPressed = true;
    buttonPressStart = millis();
  } else if(buttonState == LOW && buttonPressed) {
    // Button being held
    if(millis() - buttonPressStart >= HOLD_TIME && !modeJustChanged) {
      // Toggle mode
      if(currentMode == RECEIVE_MODE) {
        currentMode = SEND_MODE;
        lcd.clear();
        lcd.print("Mode: SEND");
        lcd.setCursor(0,1);
        if(btConnected) {
          lcd.print("BT Ready!");
        } else {
          lcd.print("Connect BT app");
        }
      } else {
        currentMode = RECEIVE_MODE;
        lcd.clear();
        lcd.print("Mode: RECEIVE");
        lcd.setCursor(0,1);
        lcd.print("Waiting...");
      }
      modeJustChanged = true;
      delay(500); // Debounce
    }
  } else if(buttonState == HIGH && buttonPressed) {
    // Button released
    buttonPressed = false;
    modeJustChanged = false;
  }
  
  // ===== BLUETOOTH CONNECTION STATUS =====
  // Connection status is now handled by callback
  // Just update btConnected flag from hasClient for safety
  btConnected = SerialBT.hasClient();
  
  // ===== SEND MODE OPERATION =====
  if(currentMode == SEND_MODE) {
    // Check for Bluetooth data
    if(SerialBT.available()) {
      String incomingData = SerialBT.readStringUntil('\n');
      incomingData.trim();
      
      if(incomingData.length() > 0) {
        btDataToSend = incomingData;
        
        // Acknowledge receipt
        SerialBT.println("Data received: " + btDataToSend);
        SerialBT.println("Transmitting via LED...");
        
        // Transmit via LED
        transmitMessage(btDataToSend);
        
        // Notify completion
        SerialBT.println("Transmission complete!");
        
        // Reset and wait for next message
        btDataToSend = "";
        lcd.clear();
        lcd.print("Mode: SEND");
        lcd.setCursor(0,1);
        lcd.print("Ready for data");
      }
    } else if(btConnected) {
      // Show ready status occasionally
      static unsigned long lastUpdate = 0;
      if(millis() - lastUpdate > 5000) {
        lcd.clear();
        lcd.print("Mode: SEND");
        lcd.setCursor(0,1);
        lcd.print("Ready for data");
        lastUpdate = millis();
      }
    }
  }
  
  // ===== RECEIVE MODE OPERATION =====
  else if(currentMode == RECEIVE_MODE) {
    static int lastState = 0; // 0=OFF, 1=ON
    static unsigned long stateStart = millis();
    static unsigned long lastActivityTime = millis();
    static bool receiving = false;
    static bool dataDisplayed = false;
    static String finalMessage = "";
    static unsigned long scrollTimer = 0;
    static int scrollPosition = 0;

    int ldrVal = analogRead(LDR_PIN);
    int currentState = (ldrVal > THRESHOLD) ? 1 : 0;
    unsigned long now = millis();

    // Detect state change
    if(currentState != lastState) {
      unsigned long duration = now - stateStart;

      if(lastState == 1) {
        // Flash ended → dot or dash
        if(duration >= DOT_DURATION/2 && duration < (DOT_DURATION + DASH_DURATION)/2) {
          morseBuffer += ".";
        } else if(duration >= (DOT_DURATION + DASH_DURATION)/2) {
          morseBuffer += "-";
        }
        // Ignore very short pulses (noise)
        
        receiving = true;
        lastActivityTime = now;
        
      } else if(lastState == 0) {
        // Gap ended
        if(duration > WORD_PAUSE) {
          // Complete current letter if any
          if(morseBuffer.length() > 0){
            char c = decodeMorse(morseBuffer);
            textBuffer += c;
            morseBuffer = "";
          }
          textBuffer += " "; // word space
        } else if(duration > LETTER_PAUSE && morseBuffer.length() > 0) {
          // Complete current letter
          char c = decodeMorse(morseBuffer);
          textBuffer += c;
          morseBuffer = "";
        }
      }

      // Show receiving message
      if(receiving && !dataDisplayed && textBuffer.length() > 0){
        lcd.clear();
        lcd.print("Receiving data...");
      }

      lastState = currentState;
      stateStart = now;
    }

    // Check for end of transmission (timeout after last activity)
    if(receiving && !dataDisplayed && (now - lastActivityTime > TIMEOUT_PAUSE)) {
      // Complete any remaining morse code
      if(morseBuffer.length() > 0) {
        char c = decodeMorse(morseBuffer);
        textBuffer += c;
        morseBuffer = "";
      }
      
      // Display final message and keep it
      if(textBuffer.length() > 0) {
        finalMessage = textBuffer;
        dataDisplayed = true;
        scrollTimer = millis();
        scrollPosition = 0;
        
        // Initial display
        scrollText(finalMessage);
      }
    }
    
    // Continuous scrolling for long messages
    if(dataDisplayed && finalMessage.length() > 16) {
      if(millis() - scrollTimer > 350) {
        scrollTimer = millis();
        
        // Top line: Static label
        lcd.setCursor(0,0);
        lcd.print("Received Msg:   ");
        
        // Bottom line: Scrolling message
        lcd.setCursor(0,1);
        
        int displayPos = scrollPosition % (finalMessage.length() + 3);
        String displayText = finalMessage + "   " + finalMessage; // Loop the text
        
        lcd.print(displayText.substring(displayPos, min(displayPos+16, (int)displayText.length())));
        
        scrollPosition++;
        if(scrollPosition >= finalMessage.length() + 3) scrollPosition = 0;
      }
    } else if(dataDisplayed && finalMessage.length() <= 16) {
      // Static display for short messages
      if(scrollPosition == 0) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Received Msg:");
        lcd.setCursor(0,1);
        lcd.print(finalMessage);
        scrollPosition = 1; // Flag to prevent redrawing
      }
    }
  }
}
