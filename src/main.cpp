#include <Arduino.h>
#include <BleKeyboard.h>

#define LDR_PIN   34
#define LED_PIN   2
#define LED_PIN2  4

int baseline = 0;
int thresholdOffset = 200;
bool calibrated = false;

unsigned long lastJumpTime = 0;
const unsigned long cooldownTime = 150; // ms (adjust)

BleKeyboard bleKeyboard("ESP32 Dino", "ESP32", 100);

void setup() {
  Serial.begin(115200);                                        
      
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);

  bleKeyboard.begin();
}

void loop() {                                                                                                                                                                                           
  if (!bleKeyboard.isConnected()) return;   

  int ldrValue = analogRead(LDR_PIN);                                                                                                                          
  

  if (!calibrated) {
    baseline = ldrValue;
    calibrated = true;
    delay(1000);
    return;
  }

      Serial.println("LDR Value: " + String(ldrValue) + " | Baseline: " + String(baseline));

  bool obstacleDetected = 
      (ldrValue < (baseline - thresholdOffset) ||
       ldrValue > (baseline + thresholdOffset));
  
  if (obstacleDetected) {
    
                         
    if (millis() - lastJumpTime > cooldownTime) {
      
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(LED_PIN2, LOW);

      // SEND SPACE ONCE
      Serial.println("Obstacle Detected - Sending SPACE");
      bleKeyboard.press(' ');
      delay(20);
      bleKeyboard.release(' ');
                                      
      // Reset the timer for the next cooldown
      lastJumpTime = millis();
    }
  }

  // LEDs reset
  if (!obstacleDetected) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN2, HIGH);

  }
}
