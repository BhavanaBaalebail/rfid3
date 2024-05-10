#include <Ultrasonic.h>

#define motorA_pin1 9  // 1st pin //motor A
#define motorA_pin2 8
#define motorB_pin1 5  // 1st pin //motor B
#define motorB_pin2 4
#define enablePinA 17   // Enable pin for motor A
#define enablePinB 16   // Enable pin for motor B

#define button_left 15
#define button_right 18
#define button_reverse 22
#define button_speed 12
#define button_follow 19

#define max_speed 255
#define min_speed 170  

#define TRIGGER_PIN 3
#define ECHO_PIN 2

const int irSensorPinr = 20; 
const int irSensorPinl = 21;

int current_speed = min_speed; // Initial speed set to minimum

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);

void setup() {
  pinMode(motorA_pin1, OUTPUT);
  pinMode(motorA_pin2, OUTPUT);
  pinMode(motorB_pin1, OUTPUT);
  pinMode(motorB_pin2, OUTPUT);
  pinMode(enablePinA, OUTPUT);
  pinMode(enablePinB, OUTPUT);

  pinMode(button_right, INPUT);
  pinMode(button_left, INPUT);
  pinMode(button_reverse, INPUT);
  pinMode(button_speed, INPUT);
  pinMode(button_follow, INPUT);

  digitalWrite(enablePinA, HIGH); 
  digitalWrite(enablePinB, HIGH); 

  pinMode(irSensorPinr, INPUT);
  pinMode(irSensorPinl, INPUT);
}

void stopMotors() {
  digitalWrite(motorA_pin1, LOW);
  digitalWrite(motorA_pin2, LOW);
  digitalWrite(motorB_pin1, LOW);
  digitalWrite(motorB_pin2, LOW);
}

void moveMotors(int speedA, int speedB, bool dirA, bool dirB) {
  analogWrite(enablePinA, speedA);
  digitalWrite(motorA_pin1, dirA ? LOW : HIGH);
  digitalWrite(motorA_pin2, dirA ? HIGH : LOW);

  analogWrite(enablePinB, speedB);
  digitalWrite(motorB_pin1, dirB ? HIGH : LOW);
  digitalWrite(motorB_pin2, dirB ? LOW : HIGH);
}

void loop() {
  int leftButtonState = digitalRead(button_left);
  int rightButtonState = digitalRead(button_right);
  int reverseButtonState = digitalRead(button_reverse);
  int speedButtonState = digitalRead(button_speed);
  int automatic = digitalRead(button_follow);

  long distance = ultrasonic.read(); 
  static int distanceCounter = 0;

  // Increase speed when speed button is pressed
  if (speedButtonState == HIGH && current_speed < max_speed) {
    current_speed += 25; // Increase speed by 25
    if (current_speed > max_speed) {
      current_speed = max_speed;
    }
    analogWrite(enablePinA, current_speed);
    analogWrite(enablePinB, current_speed);
    delay(100); // Debounce delay
  }

  // Autonomous mode
  if (automatic == HIGH) {
    int irSensorValuer = digitalRead(irSensorPinr);
    int irSensorValuel = digitalRead(irSensorPinl);

    if (distanceCounter >= 8) {
      stopMotors();
      distanceCounter = 0;
    }
    else if (irSensorValuer == HIGH && irSensorValuel == HIGH) {
      moveMotors(170, 170, HIGH, HIGH);
    } 
    else if (irSensorValuer == LOW && irSensorValuel == HIGH) {
      moveMotors(130, 130, LOW, HIGH);
    } 
    else if (irSensorValuel == LOW && irSensorValuer == HIGH) {
      moveMotors(130, 130, HIGH, LOW);
    } 
    else {
      current_speed = min_speed; // Reset speed to minimum in autonomous mode
      analogWrite(enablePinA, current_speed);
      analogWrite(enablePinB, current_speed);
      distanceCounter++;
    }

    delay(100);
  }

  else {
    if (distance <= 20) {
      if (reverseButtonState == HIGH) {
        if (leftButtonState == HIGH && rightButtonState == HIGH) 
          moveMotors(current_speed, current_speed, LOW, LOW);
        else if (leftButtonState == HIGH) 
          moveMotors(current_speed, current_speed, HIGH, LOW);
        else if (rightButtonState == HIGH) 
          moveMotors(current_speed, current_speed, LOW, HIGH);
        else 
          stopMotors();
      }
      else
        stopMotors();
    } 
    else {
      if (reverseButtonState == LOW) {
        if (leftButtonState == HIGH && rightButtonState == HIGH) {
          moveMotors(current_speed, current_speed, HIGH, HIGH);
        } else if (leftButtonState == HIGH) {
          moveMotors(current_speed, current_speed, LOW, HIGH);
        } else if (rightButtonState == HIGH) {
          moveMotors(current_speed, current_speed, HIGH, LOW);
        } else {
          stopMotors();
        }
      } 
      else if (reverseButtonState == HIGH) {
        if (leftButtonState == HIGH && rightButtonState == HIGH) {
          moveMotors(current_speed, current_speed, LOW, LOW);
        } else if (leftButtonState == HIGH) {
          moveMotors(current_speed, current_speed, HIGH, LOW);
        } else if (rightButtonState == HIGH) {
          moveMotors(current_speed, current_speed, LOW, HIGH);
        } else {
          stopMotors();
        }
      }
    }
  }
}