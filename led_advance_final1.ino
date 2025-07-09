// Pin setup
const int leds[] = {2, 3, 4};
const int buttonPin = 5;
const int potPin = A0;
const int ldrPin = A1;
const int buzzerPin = 6;
const int trigPin = A2;
const int echoPin = A3;
const int pirPin = A4;

// Variables
int pattern = 0;
bool ledState = false;
int ledIndex = 0;
unsigned long lastBlink = 0;
unsigned long lastDebounce = 0;
int lastButton = HIGH;
unsigned long lastPrint = 0;
unsigned long lastMotionTime = 0;
bool isIdle = false;

bool intruderDetected = false;
unsigned long intruderTime = 0;

void setup() {
  for (int i = 0; i < 3; i++) pinMode(leds[i], OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);

  Serial.begin(9600);
  Serial.println("System Started");
}

void loop() {
  // Motion Idle Check
  if (digitalRead(pirPin) == HIGH) {
    lastMotionTime = millis();
    if (isIdle) {
      Serial.println("Waking from idle.");
      isIdle = false;
    }
  }

  if (!isIdle && millis() - lastMotionTime > 30000) {
    Serial.println("Entering idle mode.");
    allOff();
    isIdle = true;
  }
  if (isIdle) return;

  int ldrVal = analogRead(ldrPin);
  if (ldrVal > 900) {
    allOff();
    if (millis() - lastPrint > 900) {
      Serial.print("Too Bright | LDR: ");
      Serial.println(ldrVal);
      lastPrint = millis();
    }
    return;
  }

  // Proximity-based intruder alarm
  int distance = getDistance();
  if (distance > 0 && distance < 15) {
    if (!intruderDetected) {
      intruderDetected = true;
      intruderTime = millis();
      Serial.println("🚨 INTRUDER ALERT!");
    }
  }

  if (intruderDetected) {
    alarmMode();
    if (millis() - intruderTime > 3000) {
      intruderDetected = false;
      allOff();
      noTone(buzzerPin);
      Serial.println("Alarm reset.");
    }
    return;
  }

  // Regular behavior
  int potVal = analogRead(potPin);
  int speed = map(potVal, 0, 1023, 100, 1000);

  if (distance < 30) speed = 100;

  if (millis() - lastPrint > 500) {
    Serial.print("Pattern: ");
    Serial.print(pattern);
    Serial.print(" | Dist: ");
    Serial.print(distance);
    Serial.print(" cm | LDR: ");
    Serial.print(ldrVal);
    Serial.print(" | Speed: ");
    Serial.println(speed);
    lastPrint = millis();
  }

  checkButton();
  updatePattern(pattern, speed);
}

void checkButton() {
  int reading = digitalRead(buttonPin);
  if (reading == LOW && lastButton == HIGH && millis() - lastDebounce > 50) {
    pattern = (pattern + 1) % 3;
    ledIndex = 0;
    tone(buzzerPin, 500, 100);
    Serial.print("Switched to pattern ");
    Serial.println(pattern);
    lastDebounce = millis();
  }
  lastButton = reading;
}

void updatePattern(int p, int d) {
  if (millis() - lastBlink >= d) {
    lastBlink = millis();
    allOff();

    if (p == 0) {
      digitalWrite(leds[ledIndex], HIGH);
      ledIndex = (ledIndex + 1) % 3;
    } else if (p == 1) {
      digitalWrite(leds[2 - ledIndex], HIGH);
      ledIndex = (ledIndex + 1) % 3;
    } else {
      ledState = !ledState;
      for (int i = 0; i < 3; i++)
        digitalWrite(leds[i], ledState ? HIGH : LOW);
    }
  }
}

void allOff() {
  for (int i = 0; i < 3; i++) digitalWrite(leds[i], LOW);
}

void alarmMode() {
  static bool state = false;
  if (millis() - lastBlink > 100) {
    lastBlink = millis();
    state = !state;
    for (int i = 0; i < 3; i++)
      digitalWrite(leds[i], state ? HIGH : LOW);
    tone(buzzerPin, 1000);
  }
}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.034 / 2;
}


