#include <Servo.h>

const int NUMBER_OF_SERVOS = 10;

const int THUMB = 14;
const int INDEX = 15;
const int MIDDL = 16;
const int RINGF = 17;
const int PINKY = 18;

const int CLAVI =  9;
const int SHYAW = 10;
const int SHPIT = 11;
const int BICEP = 12;
const int WRIST = 13;

Servo allServos[NUMBER_OF_SERVOS];

int incomingByte = 0;

void setup() {
  Serial.begin(115200);
  allServos[0].attach(THUMB);
  allServos[1].attach(INDEX);
  allServos[2].attach(MIDDL);
  allServos[3].attach(RINGF);
  allServos[4].attach(PINKY);
  allServos[5].attach(BICEP);
  allServos[6].attach(SHYAW);
  allServos[7].attach(SHPIT);
  allServos[8].attach(CLAVI);
  allServos[9].attach(WRIST);
}

void loop() {
  int count = 0;
  int i;

  for (i = 0; i < NUMBER_OF_SERVOS; i++) {
    
    /* wait for byte and read it */
    while(Serial.available() == 0);
    incomingByte = Serial.read();
    
    /*
      will get a number between 0 and 180 for fingers, 0 is open and 180 is closed
      thumb, index and middle are connected backwards
      the Leap reader will take care of the values for the arm servos
    */
    if (i == 0 || i == 1 || i == 2) {
      allServos[i].write(180 - incomingByte);
    } else {
      allServos[i].write(incomingByte);
    }
  }
}
