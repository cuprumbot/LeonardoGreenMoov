#include <Servo.h>

const int NUMBER_OF_SERVOS = 7;
Servo allServos[NUMBER_OF_SERVOS];

int incomingByte = 0;

void setup() {
  Serial.begin(115200);
  allServos[0].attach( 7);  //interior de cabeza (yaw)
  allServos[1].attach( 6);  //piston del cuello (pitch)
  
  /* dedos: reciben 180 para estirar y 0 para contraer */
  allServos[2].attach(14);  //pulgar
  allServos[3].attach(15);  //indice
  allServos[4].attach(16);  //medio
  allServos[5].attach(17);  //anular
  allServos[6].attach(18);  //menique
}

void loop() {
  int count = 0;
  int i;

  for (i = 0; i < NUMBER_OF_SERVOS; i++) {
    while(Serial.available() == 0);
    incomingByte = Serial.read();
    
    if (i == 2 || i == 3 || i == 4) {  //dedos: pulgar, indice y medio tienen servo al reves
      allServos[i].write(180 - incomingByte);
    } else {
      allServos[i].write(incomingByte);
    }
  }
}
