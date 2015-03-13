/* johnny-five, Arduino y servos */
var five        = require('johnny-five');
var board       = new five.Board();
var servo0;     //pulgar
var servo1;     //indice
var servo2;     //medio
var servo3;     //anular
var servo4;     //menique

/* websocket para comunicacion desde el Leap Motion */
var webSocket   = require('ws');
var ws          = new webSocket('ws://127.0.0.1:6437');

/* identificadores para los dedos */
var idMano      = 0;
var idDedo      = 0;
var extendidos  = [false, false, false, false, false];
var ready       = false;
var ABIERTO     = 180;
var CERRADO     = 0;

board.on('ready', function() {
    servo0  = new five.Servo( 9);
    servo1  = new five.Servo(10);
    servo2  = new five.Servo(11);
    servo3  = new five.Servo(12);
    servo4  = new five.Servo(13);

    ready = true;
});

ws.on('message', function(data, flags) {
	
    frame = JSON.parse(data);
    extendidos = [false, false, false, false, false];

    // comprobar que exista una mano y dedos
    if (frame.hands && frame.hands.length == 1 && frame.pointables && frame.pointables.length > 0) {

        hand = frame.hands[0];
        idMano = hand.id;

        for (i = 0; i < frame.pointables.length; i++) {
            finger = frame.pointables[i];
            idDedo = finger.id;

            index = idDedo - (idMano * 10);

            extendidos[index] = true;
        }
    }

    movimiento(extendidos);
});


function movimiento (dedos) {
    if (ready) {
        servo0.to( (dedos[0]) ? ABIERTO : CERRADO );
        servo1.to( (dedos[1]) ? ABIERTO : CERRADO );
        servo2.to( (dedos[2]) ? ABIERTO : CERRADO );
        servo3.to( (dedos[3]) ? ABIERTO : CERRADO );
        servo4.to( (dedos[4]) ? ABIERTO : CERRADO );
    }
    console.log(dedos);
};