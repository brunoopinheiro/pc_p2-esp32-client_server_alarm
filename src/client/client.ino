#include <ESP32Servo.h>

#define G 18
#define F 5
#define A 17
#define B 16
#define DP 34
#define C 12
#define D 32
#define E 25
#define NOTE_D4 294
#define SERVO_PORT 33

enum State {
  DEACTIVATED = 0,
  PRE_ACTIVATION = 1,
  ACTIVATED = 2,
  PRE_PASSWORD = 3,
  PASSWORD_STATE = 4,
  LOCKDOWN = 5,
};

const int sensorPin = 15;
const int buttonPin = 23;
const int buzzerPin = 14;
const int ledPin = 21;
const String password = "senha";
const int servoLowered = 10;
const int servoLifted = 100;
bool displayMap[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // digit 0
  {0, 1, 1, 0, 0, 0, 0}, // digit 1
  {1, 1, 0, 1, 1, 0, 1}, // digit 2
  {1, 1, 1, 1, 0, 0, 1}, // digit 3
  {0, 1, 1, 0, 0, 1, 1}, // digit 4
  {1, 0, 1, 1, 0, 1, 1}, // digit 5
  {1, 0, 1, 1, 1, 1, 1}, // digit 6
  {1, 1, 1, 0, 0, 0, 0}, // digit 7
  {1, 1, 1, 1, 1, 1, 1}, // digit 8
  {1, 1, 1, 1, 0, 1, 1}, // digit 9
};
int buttonState = LOW;
int passwordTries = 0;
Servo servoMotor;
int servoPos = 0;
State state;

void writeNumber(int number) {
  bool* display = displayMap[number];

  digitalWrite(A, display[0]);
  digitalWrite(B, display[1]);
  digitalWrite(C, display[2]);
  digitalWrite(D, display[3]);
  digitalWrite(E, display[4]);
  digitalWrite(F, display[5]);
  digitalWrite(G, display[6]);
}

void turnOffDisplay() {
  digitalWrite(A, 0);
  digitalWrite(B, 0);
  digitalWrite(C, 0);
  digitalWrite(D, 0);
  digitalWrite(E, 0);
  digitalWrite(F, 0);
  digitalWrite(G, 0);
  digitalWrite(DP, 1);
}

void blinkLed(int time) {
  for (int i = 0; i < time; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

void checkPassword(String inputPassword) {
  inputPassword.trim();
  if (!password.compareTo(inputPassword)) {
    Serial.println("PASSWORD CORRECT: System Deactivated");
    blinkLed(2);
    state = DEACTIVATED;
    return;
  } else {
    Serial.println("PASSWORD INCORRECT: Try Again");
    passwordTries += 1;
  }
}

void readPasswordInput() {
  String inputPassword = "";
  if (Serial.available() > 0) {
    inputPassword = Serial.readString();
    checkPassword(inputPassword);
  }
}

void countdown(int initialNumber) {
  const int initialPwdTry = passwordTries;
  if (initialNumber > 9) {
    initialNumber = 9;
  }
  Serial.println("INPUT YOUR PASSWORD: ");
  while (initialNumber >= 0) {
    writeNumber(initialNumber);
    readPasswordInput();
    delay(1000);
    initialNumber--;
    if (passwordTries != initialPwdTry) {
      return;
    }
    if (state == DEACTIVATED) {
      return;
    }
  }
  turnOffDisplay();
  passwordTries += 1;
}

void beepBuzzer(int repetitions) {
  for (int i = 0; i < repetitions; i++) {
    tone(buzzerPin, NOTE_D4, 1000/4);
    delay(500);
  }
}

void servoLift() {
  Serial.println("LIFTING LOCK");
  for (servoPos = servoLowered; servoPos <= servoLifted; servoPos +=1) {
    servoMotor.write(servoPos);
    delay(15);
  }
}

void servoLower() {
  Serial.println("LOWERING LOCK");
  for (servoPos = servoLifted; servoPos >= servoLowered; servoPos -= 1) {
    servoMotor.write(servoPos);
    delay(15);
  }
}

bool sensorFeedback() {
  int sensorValue = analogRead(sensorPin);
  float lightDetectedAmount = map(sensorValue, 0, 1023, 0, 10);
  if (lightDetectedAmount > 3) {
    Serial.println("WARNING: Light Detected");
    return true;
  }
  return false;
}

// REGION: states macro functions

void deactivated() {
  buttonState = LOW;
  passwordTries = 0;
  digitalWrite(ledPin, LOW);
  turnOffDisplay();
  if (servoPos < 90) {
    servoLift();
  }
  buttonState = digitalRead(buttonPin);
  if (buttonState == 1) {
    state = PRE_ACTIVATION;
  }
}

void preActivation() {
  Serial.println("SYSTEM ACTIVATING IN 10 SECONDS...");
  blinkLed(10);
  servoLower();
  state = ACTIVATED;
}

void activated() {
  digitalWrite(ledPin, LOW);
  turnOffDisplay();
  bool light = sensorFeedback();
  if (light == true) {
    state = PRE_PASSWORD;
  }
}

void prePassword() {
  digitalWrite(ledPin, LOW);
  beepBuzzer(2);
  state = PASSWORD_STATE;
}

void passwordState() {
  if (passwordTries >= 2) {
    state = LOCKDOWN;
    return;
  }
  digitalWrite(ledPin, LOW);
  countdown(10);
}

void lockdown() {
  while (true) {
    tone(buzzerPin, NOTE_D4, 750);
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(250);
  }
}

// REGION: Setup and Main Loop

void setup() {
  // put your setup code here, to run once:
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(DP, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  servoMotor.attach(SERVO_PORT);
  servoLift();
  Serial.begin(115200);
  Serial.println("Starting...");
  blinkLed(2);
  state = DEACTIVATED;
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case DEACTIVATED:
      deactivated();
      break;
    case PRE_ACTIVATION:
      preActivation();
      break;
    case ACTIVATED:
      activated();
      break;
    case PRE_PASSWORD:
      prePassword();
      break;
    case PASSWORD_STATE:
      passwordState();
      break;
    case LOCKDOWN:
      lockdown();
      break;
  }
}
