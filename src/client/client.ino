#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

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

const char* ssid = "WIFI";
const char* wifiPassword = "PASSWORD";
const String serverBaseURL = "IP:PORT";
const String username = "esp";
const int sensorPin = 2;
const int buttonPin = 23;
const int buzzerPin = 14;
const int touchPin = 4;
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

bool connectWifi() {
  WiFi.begin(ssid, wifiPassword);
  Serial.println("WIFI: Connecting...");
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    count += 1;
    if (count >= 100) {
      return false;
    }
  }
  Serial.println("");
  return true;
}

void checkServer(String serverURL) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  HTTPClient http;
  http.begin(serverURL.c_str());
  int responseStatus = http.GET();

  if (responseStatus == 200) {
    Serial.print("HTTP Response Code: ");
    Serial.println(responseStatus);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("HTTP Response Code: ");
    Serial.println(responseStatus);
  }
  http.end();
}

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

bool authRequest(String inputPassword) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  HTTPClient http;
  String url = String(serverBaseURL + "/login/");
  http.begin(url.c_str());
  http.addHeader("Content-Type", "application/json");

  String payload = String("{\"username\": \"" + username + "\", \"password\": \"" + inputPassword + "\"}");
  int responseStatus = http.POST(payload);
  if (responseStatus == 200) {
    Serial.print("HTTP Response Code: ");
    Serial.println(responseStatus);
    String payload = http.getString();
    Serial.println(payload);
    http.end();
    return true;
  } else {
    Serial.print("HTTP Response Code: ");
    Serial.println(responseStatus);
    http.end();
    return false;
  }
  return false;
}

void checkPassword(String inputPassword) {
  inputPassword.trim();
  bool res = authRequest(inputPassword);
  if (res == true) {
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
  servoMotor.attach(SERVO_PORT);
  Serial.println("LIFTING LOCK");
  for (servoPos = servoLowered; servoPos <= servoLifted; servoPos +=1) {
    servoMotor.write(servoPos);
    delay(15);
  }
  servoMotor.detach();
}

void servoLower() {
  servoMotor.attach(SERVO_PORT);
  Serial.println("LOWERING LOCK");
  for (servoPos = servoLifted; servoPos >= servoLowered; servoPos -= 1) {
    servoMotor.write(servoPos);
    delay(15);
  }
  servoMotor.detach();
}

bool touchFeedback() {
  int touch = touchRead(touchPin);
  if (touch <= 10) {
    return true;
  }
  return false;
}

bool sensorFeedback() {
  int sensorValue = analogRead(sensorPin);
  float lightDetectedAmount = map(sensorValue, 0, 1023, 0, 10);
  if (lightDetectedAmount > 4) {
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
  bool touch = touchFeedback();
  if (touch == true || light == true) {
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
  servoLift();
  Serial.begin(115200);
  Serial.println("Starting...");
  connectWifi();
  blinkLed(2);
  checkServer(serverBaseURL);
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
