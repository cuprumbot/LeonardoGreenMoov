#include <Servo.h>

Servo yaw;
Servo pitch;

int incomingByte = 0;

void setup() {
  Serial.begin(115200);
  yaw.attach(11);
  pitch.attach(10);
}

void loop() {
  int count = 0;
        
  /* Yaw */
  while (Serial.available() == 0);
  incomingByte = Serial.read();
  //analogWrite(10, incomingByte);
  yaw.write(incomingByte);
  
  /* Pitch */
  while (Serial.available() == 0);
  incomingByte = Serial.read();
  //analogWrite(11, incomingByte);
  pitch.write(incomingByte);
}
