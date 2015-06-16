#include <Servo.h>

const int NUMBER_OF_SERVOS = 12;

const int HDPIT =  6;
const int HDYAW =  7;

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
  allServos[0].attach(HDYAW);
  allServos[1].attach(HDPIT);
  allServos[2].attach(THUMB);
  allServos[3].attach(INDEX);
  allServos[4].attach(MIDDL);
  allServos[5].attach(RINGF);
  allServos[6].attach(PINKY);
  allServos[7].attach(BICEP);
  allServos[8].attach(SHYAW);
  allServos[9].attach(SHPIT);
  allServos[10].attach(CLAVI);
  allServos[11].attach(WRIST);
}

void loop() {
  int count = 0;
  int i;

  for (i = 0; i < NUMBER_OF_SERVOS; i++) {
    while(Serial.available() == 0);
    incomingByte = Serial.read();
    
    /*
      will get a number between 0 and 180 for fingers, 0 is open and 180 is closed
      thumb, index and middle are connected backwards
      the Leap reader will take care of the values for the arm servos
    */
    if (i == 2 || i == 3 || i == 4) {
      allServos[i].write(180 - incomingByte);
    } else {
      allServos[i].write(incomingByte);
    }
  }
}
