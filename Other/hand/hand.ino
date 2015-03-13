#include <Servo.h>

Servo fingers[5];

int incomingByte = 0;

void setup() {
  Serial.begin(115200);
  fingers[0].attach( 9);
  fingers[1].attach(10);
  fingers[2].attach(11);
  fingers[3].attach(12);
  fingers[4].attach(13);
}

void loop() {
  int count = 0;
  int i;

  for (i = 0; i < 5; i++) {
    while(Serial.available() == 0);
    incomingByte = Serial.read();
    if (i == 0 || i == 1 || i == 2) {
      fingers[i].write(180 - incomingByte);
    } else {
      fingers[i].write(incomingByte);
    }
  }
}
