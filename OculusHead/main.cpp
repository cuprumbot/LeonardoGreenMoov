#include <iostream>
#include <string.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <string.h>
#include <thread>
#include <chrono>
#include <math.h>

/* VR */
#include "OVR.h"

/* CV */
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

/* Serial */
#include "Serial.cpp"

/* Leap */
#include "Leap.h"

#define UNLOCK_LIMIT_LEFT		150
#define UNLOCK_LEFT				190
#define UNLOCK_LIMIT_RIGHT		 30
#define UNLOCK_RIGHT			350
#define HARDLOCK_LIMIT_LEFT		130
#define HARDLOCK_LIMIT_RIGHT	 50

#define REST_ARMPITCH			 80
#define REST_ARMYAW				 90
#define REST_ARMANGLE			 90
#define REST_SHOULDERANGLE		 20
#define REST_HANDROLLANGLE		 0

using namespace Leap;
using namespace cv;
using namespace std;

/* enable devices */
bool debugPrint = true;
bool cameraEnabled	= true;
bool oculusEnabled	= true;
bool leapEnabled	= true;
bool arduinoEnabled = true;

/* VR */
ovrHmd hmd;					//handler for the headset
String movStatus = "";

/* CV */
VideoCapture capL(0);		//capture cameras
VideoCapture capR(1);
int		capWidth   = 1080;	//size for capturing
int		capHeight  = 720;
int		showWidth  = 1000;	//size for showing
int		showHeight = 960;
double	realWidthL  = 0;
double	realHeightL = 0;
double	realWidthR  = 0;
double	realHeightR = 0;
int		fps = 60;

/* Serial */
Serial* SP;

/* Leap */
int  originalAngles[] = {180,180,180,180,180};
char fingerAngles[] = {180,180,180,180,180};
char armAngles[] = {REST_ARMPITCH, REST_ARMYAW, REST_ARMANGLE, REST_SHOULDERANGLE, REST_HANDROLLANGLE};
bool isHandPresent = false;

/*
	----------------------------------------
	CLASE LeapListener
	Contiene los listeners necesarios para las acciones aceptadas por el Leap Motion
	----------------------------------------
*/
class LeapListener : public Listener {
	virtual void onFrame(const Controller&);
};

const std::string fingerNames[]	= {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[]	= {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[]	= {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

void LeapListener::onFrame(const Controller& controller) {
	// Get the most recent frame and report some basic information
	std::this_thread::sleep_for(std::chrono::milliseconds(30));		//TO DO: check this sleep
	
	const Frame frame = controller.frame();

	HandList hands = frame.hands();
	//Default position
	//bicep
	int armAngle = REST_ARMPITCH;
	//elbow
	int armPitchAngle = REST_ARMYAW;
	//yaw
	int armYawAngle = REST_ARMPITCH;
	//etc
	int shoulderAngle = REST_SHOULDERANGLE;
	int handRollAngle = REST_HANDROLLANGLE;
	//Para escribir al arduino
	char c, c1, c2, c3, c4, c5;

	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		isHandPresent = true;
		// Get the first hand
		const Hand hand = *hl;
		std::string handType = hand.isLeft() ? "Left hand" : "Right hand";

		// Get the hand's normal vector and direction
		const Leap::Vector normal = hand.palmNormal();
		const Leap::Vector direction = hand.direction();
	
		// Get the Arm bone
		Arm arm = hand.arm();
		float aux = 0;

		//Arm 45 - 175	
		float handposz = hand.palmPosition()[2];
		float lengthBicep = 250;
		aux = handposz/lengthBicep;		//Por si la division no esta en [-1, 1]
		if(aux > 1) {
			armAngle = 45;
			cout << "CHECK HANDPOSZ 45" << endl;
		} else if (aux < -1) {
			armAngle = 175;
			cout << "CHECK HANDPOSZ 175" << endl;
		} else {
			armAngle = asin(handposz/lengthBicep)*180/PI;	
		}

		//Da angulos de -90 a 90
		if(armAngle < 0){
			armAngle = abs(armAngle) + 90;				//Para enfrente es negativo por lo tanto volverlo positivo y sumarte 90 del otro plano
		} else {
			armAngle = 90 - armAngle;					//Ajustar el angulo porque da el complemento
		}
	
		//Elbow 5 - 95
		armPitchAngle = arm.direction().pitch()*180/PI;
		//Da angulos de -90 a 90
		if (armPitchAngle < 0) {
			armPitchAngle = 90 - abs(armPitchAngle);	//Negativo es abajo y da el complemento
		} else {
			armPitchAngle = armPitchAngle + 90;			//Positivo mas 90 de los negativos
		}

		if(armAngle > 89) {
			armPitchAngle = armPitchAngle - (armAngle - 90);	//Al levantar el brazo el angulo del codo cambia
																//-90 porque en 90 el codo esta normal en la posicion 90 del brazo
		}		
		if(armPitchAngle > 95) {
			armPitchAngle = 95;
		}
		if(armPitchAngle < 5) {
			armPitchAngle = 5;
		}
	
		//Bicep rot 20 - 180
		armYawAngle = arm.direction().yaw()*180/PI;
		armYawAngle = (65 + armYawAngle)*1.12;			//95 es el centro, y factor multiplicativo

		if(armYawAngle > 180) {
			armYawAngle = 180;
		}
		if(armYawAngle < 20) {
			armYawAngle = 20;
		}
		armYawAngle = 180 - armYawAngle;				//Darle vuelta al angulo

		//Shoulder 20 - 80 
		int elbowposx =  arm.elbowPosition()[0];		//Posicion del codo en x esta entre [-300,-200]
		aux = abs(elbowposx) - 200;						//Volverlo un valor entre [0, 100]
		if(aux < 0) {
			aux = 0;
		} else if (aux > 100) {
			aux = 100;
		}
		shoulderAngle = aux * 0.8;					//Factor multiplicativo para que quede entre [30,80]
		if(shoulderAngle < 20){
			shoulderAngle = 20;
		} else if (shoulderAngle > 80) {
			shoulderAngle = 80;		
		} // BROLO CHANGED A LOT OF STUFF HERE

		//Hand roll 0 - 180
		handRollAngle = hand.palmNormal().roll()*180/PI;
		if(handRollAngle < 0){							//Ignorar la parte negativa 
			handRollAngle = abs(handRollAngle);
			if(handRollAngle < 91) {
				handRollAngle = 0;
			} else {
				handRollAngle = 180;
			}
		}
		handRollAngle = 180 - handRollAngle;			//Complemento del angulo
		
		// Get fingers
		const FingerList fingers = hand.fingers();
		int counter = 0;
			
		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			const Finger finger = *fl;

			// Get finger bones
			Leap::Vector vectors[3];	
			int angle;
			
				//Version abierto cerrado
				if(finger.isExtended()) {
					angle = 0;
				} else {
					angle = 180;
				}

				originalAngles[counter] = angle;

				c = (char)(angle);
				fingerAngles[counter] = c;

				counter++;
		} //for end: FingerList iterator
		
		c1 = (char)(armPitchAngle);
		c2 = (char)(armYawAngle);
		c3 = (char)(armAngle);
		c4 = (char)(shoulderAngle);
		c5 = (char)(handRollAngle);

		armAngles[0] = c1;
		armAngles[1] = c2;
		armAngles[2] = c3;
		armAngles[3] = c4;
		armAngles[4] = c5;
	} //for end: HandList iterator

	if (frame.hands().isEmpty()) {
		//cout << "Look mom! No hands!" << endl;
		isHandPresent = false;
		fingerAngles[0] = 180;
		fingerAngles[1] = 180;
		fingerAngles[2] = 180;
		fingerAngles[3] = 180;
		fingerAngles[4] = 180;
		
		armAngles[0] = REST_ARMPITCH;
		armAngles[1] = REST_ARMYAW;
		armAngles[2] = REST_ARMANGLE;
		armAngles[3] = REST_SHOULDERANGLE;
		armAngles[4] = REST_HANDROLLANGLE;
	}
}
/*
	----------------------------------------
	FIN DE LA CLASE LeapListener
	----------------------------------------
*/

/* Leap */
LeapListener listener;
Controller	 controller;


/*
	----------------------------------------
	INITIALIZATION
	----------------------------------------
*/
int Init () {

	/*	================================================================================
		OCULUS
		Initialize and configure the Oculus
		================================================================================ */
	if (oculusEnabled) {
		ovr_Initialize();
		hmd = ovrHmd_Create(0);
		ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
	}

	/*	================================================================================
		CAMERA
		Configure OpenCV use for capturing the cameras
		================================================================================ */
	if (cameraEnabled) {

		/* Configure camera capture size */
		capL.set(CV_CAP_PROP_FRAME_WIDTH,  capWidth);
		capL.set(CV_CAP_PROP_FRAME_HEIGHT, capHeight);
		capR.set(CV_CAP_PROP_FRAME_WIDTH,  capWidth);
		capR.set(CV_CAP_PROP_FRAME_HEIGHT, capHeight);
		
		/* Check if cameras are open */
		if (!capL.isOpened() || !capR.isOpened()) {
			std::cout << "Cannot open the video camera" << std::endl;
			return -1;
		}

		/* Get the real sizes from cameras */
		realWidthL  = capL.get(CV_CAP_PROP_FRAME_WIDTH);
		realHeightL = capL.get(CV_CAP_PROP_FRAME_HEIGHT);
		realWidthR  = capR.get(CV_CAP_PROP_FRAME_WIDTH);
		realHeightR = capR.get(CV_CAP_PROP_FRAME_HEIGHT);
		
		/* Windows for both cameras */
		namedWindow("inMoov",CV_WINDOW_NORMAL);
		cvSetWindowProperty( "inMoov", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN );
		cvResizeWindow("inMoov", 1080, 1920);
		cvMoveWindow("inMoov", 1601, 0);		//The Oculus is set as an extended monitor
												//My main monitor is 1600px in width, with this position the new window is forced to the second monitor

		/* Window for main monitor */
		namedWindow("main monitor",CV_WINDOW_AUTOSIZE);
		cvMoveWindow("main monitor", 100, 100);

		if (debugPrint) {
			cout << "Frame size 1: " << realWidthL << " x " << realHeightL << endl;
			cout << "Frame size 2: " << realWidthR << " x " << realHeightR << endl;
		}
	}


	/*	================================================================================
		ARDUINO
		Connect to serial to comunicate with Arduino
		================================================================================ */
	if (arduinoEnabled) {
		SP = new Serial("\\\\.\\COM13");
		//SP = new Serial("COM4");				// TO DO: Cambiar el nombre a minusculas

		if (SP->IsConnected() == false) return -2;
		else if (debugPrint) cout << "Serial OK!" << endl;
	}

	/*	================================================================================
		LEAP
		Add listener for the Leap
		================================================================================ */
	if (leapEnabled) {
		controller.addListener(listener);
	}

	/* Success */
	return 1;
}

int Loop () {
	Mat frameL, frameR;
	Mat composition;

	int delay = 15;
	int frame = 0;
	int counter = 10;

	char e = 0;
	char c = 90;
	char cYaw = 90;
	char cPitch = 90;
	int origYaw = 90;		//Original center
	int readYaw = 90;		//Most recently read
	int currYaw = 90;		//To be reported via Serial
	int lastYaw = 90;		//Last valid
	int lockYaw = 90;		//Check before unlocking
	bool firstYaw = true;
	bool locked = false;
	bool hardLocked = false;
	int hardStatus = 0;

	String s;

	bool bSuccessL;
	bool bSuccessR;

	while (true) {
		system("cls");

		/*	================================================================================
			OCULUS
			Read position and save angles of headset to variables
			================================================================================ */
		if (oculusEnabled) {
		
			/* Get orientation as a Quaternion */
			ovrHmd_BeginFrameTiming(hmd, frame);
			ovrTrackingState trackState = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
			ovrQuatf curOrient = trackState.HeadPose.ThePose.Orientation;
			ovrHmd_EndFrameTiming(hmd);
			frame++;

			/* Convert to Euler angles */
			float eyeYaw, eyePitch, eyeRoll; 
			OVR::Quatf l_Orientation = OVR::Quatf(curOrient);
			l_Orientation.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&eyeYaw, &eyePitch, &eyeRoll);
			eyeYaw		= OVR::RadToDegree(eyeYaw);
			eyePitch	= OVR::RadToDegree(eyePitch);
			eyeRoll		= OVR::RadToDegree(eyeRoll);
	
			/* Pitch */
			cPitch	= (char)(eyePitch + 90);

			/* Yaw */
			/* Gimbal lock on pitches 0 and 180, when this happens don't change yaw*/
			if ((eyePitch+90) > 10 && (eyePitch+90) < 170) {
				//cYaw = (char)eyeYaw;

				if (firstYaw) {

					origYaw = (int)eyeYaw;
					if (eyeYaw != 0) {
						firstYaw = false;										//Stop once we get the first valid reading
						readYaw = 90;
						movStatus = "";
					}

				} else {
					
					if (!locked) lastYaw = readYaw;								//Save prev position
					else lockYaw = readYaw;

					readYaw = (int)((int)(eyeYaw + 360 + 90 - origYaw) % 360);	//Read current position
																				//+360 : make it positive, +90 : center it, -origYaw : compare it
					
					if (hardLocked) {

						cout << "HARD LOCKED" << lockYaw << endl;
						
						if (hardStatus == 1 && (eyePitch+90) > 135) {
							cout << "FIRST STEP" << endl;
							hardStatus = 2;
						} else if (hardStatus == 2 && (eyePitch+90) < 45) {
							cout << "SECOND STEP" << endl;
							hardStatus = 3;
						} else if (hardStatus == 3 && (eyePitch+90) > 135) {
							cout << "HARD LOCK REMOVED" << endl;
							hardLocked = false;
							locked = false;
							movStatus = "";
							readYaw = 90;		//Most recently read
							currYaw = 90;		//To be reported via Serial
							lastYaw = 90;		//Last valid
							lockYaw = 90;		//Check before unlocking
						}

					} else if (locked) {

						cout << "LOCKED" << endl;

						// It was returned to where it locked, unlock it
						if (	(currYaw == 0 && (readYaw < UNLOCK_LIMIT_RIGHT || readYaw > UNLOCK_RIGHT))
							||	(currYaw == 180 && (readYaw > UNLOCK_LIMIT_LEFT && readYaw < UNLOCK_LEFT))	)
						{
							locked = false;										//Unlock it
							movStatus = "";
							cout << "UNLOCKING..." << endl;
						}
						else
						{
							if ( readYaw > HARDLOCK_LIMIT_RIGHT && readYaw < HARDLOCK_LIMIT_LEFT )
							{
								
								//hardLocked = false;
								
								hardLocked = true;
								movStatus = "--- HARD LOCK ---";
								hardStatus = 1;
								currYaw = 90;									//Center head
								cPitch = (char)90;								//Override pitch
								
							}
						}

						//cout << "CURR YAW:   " << currYaw << "   LAST YAW:   " << lastYaw << "   LOCK YAW:   " << lockYaw << "   READ YAW:   " << readYaw << endl;
						cout << "last READ yaw:\t" << readYaw << "\tlast VALID yaw:\t" << currYaw << endl;

					} else {													//Not locked

						if (readYaw < 0 || readYaw > 180) {						//Went out of movement range

							locked = true;
							//locked = false;
							movStatus = "LOCKED";
							
							if (readYaw < UNLOCK_LIMIT_RIGHT || readYaw > UNLOCK_RIGHT)
								currYaw = 0;									//Right, lowest valid angle
							else if (readYaw > UNLOCK_LIMIT_LEFT && readYaw < UNLOCK_LEFT)
								currYaw = 180;									//Left, highest valid angle

						} else {												//Valid movement range
							currYaw = readYaw;
						}

						//system("cls");
						//cout << "CURR YAW:   " << currYaw << "   LAST YAW:   " << lastYaw << "   LOCK YAW:   " << lockYaw << "   READ YAW:   " << readYaw << endl;
						cout << "last READ yaw:\t\t" << readYaw << endl;
					}

					cYaw = (char)currYaw;

				}


			} else if ((eyePitch+90) >= 170) {
				cPitch = (char)180;
			} else {
				cPitch = (char)0;
			}

			if (debugPrint) {
				//cout << "Mov status: " << movStatus << endl;
			}
		}

		/*	================================================================================
			ARDUINO
			Write to serial the data from Oculus and Leap
			================================================================================ */
		if (arduinoEnabled) {
			/* head */
			SP->WriteData(&cYaw, 1);
			SP->WriteData(&cPitch, 1);
			
			for (int arrIndex = 0; arrIndex < 5; arrIndex++) {
				fingerAngles[arrIndex] = (originalAngles[arrIndex] < 70) ? 0 : 180;		
				c = fingerAngles[arrIndex];
				SP->WriteData(&c, 1);
			}
			for (int arrIndex = 0; arrIndex < 5; arrIndex++) {
				c = armAngles[arrIndex];
				SP->WriteData(&c, 1);
			}

		}

		if (debugPrint) {
				//system("cls");

				cout << "last READ pitch:\t" << (int)cPitch << endl;

				

				if (isHandPresent) {
					cout << "\nArm:\n"
					  << "Codo     \t" << (int)((armAngles[0]+255) % 255) << "\n"
					  << "Rotacion \t" << (int)((armAngles[1]+255) % 255) << "\n"	
					  << "Brazo    \t" << (int)((armAngles[2]+255) % 255) << "\n"
					  << "Hombro   \t" << (int)((armAngles[3]+255) % 255) << "\n"
					  << "Muneca   \t" << (int)((armAngles[4]+255) % 255) << "\n"
					  << std::endl;

					cout << "\nFingers:\n" 
							<< (int)fingerAngles[0] << "\t"
							<< (int)fingerAngles[1] << "\t"
							<< (int)fingerAngles[2] << "\t"
							<< (int)fingerAngles[3] << "\t"
							<< (int)fingerAngles[4] << "\t"
						<< endl;
				} else {
					cout << "\nArm: NO ARM PRESENT" << endl;
				}
			}

		/*	================================================================================
			CAMERA
			Use OpenCV to read from cameras and show image on screen
			================================================================================ */
		if (cameraEnabled) {
			/* Read from cameras */
			bSuccessL = capL.read(frameL);
			bSuccessR = capR.read(frameR);

			if (!bSuccessL || !bSuccessR) {
				cout << "Failed to read from camera" << endl;
				return -1;
			}

			/* Resize frames */
			resize(frameL, frameL, Size(showWidth,showHeight), 0, 0, INTER_CUBIC);
			resize(frameR, frameR, Size(showWidth,showHeight), 0, 0, INTER_CUBIC);
		
			/* Make composite image */
			composition = Mat(1920, 1080, frameL.type());					//create matrix to join the captured images; args: rows, columns
			Mat left  (composition, Rect(0, 0, showWidth, showHeight));		//put frames on matrix; args: x, y, w, h
			frameL.copyTo (left);
			Mat right (composition, Rect(1080-showWidth, 960, showWidth, showHeight));
			frameR.copyTo (right);

			/* Make image for main monitor */
			Mat matRotate		(850,850,frameL.type());
			Mat matMainMonitor	(850,850,frameL.type());
			resize(frameR, matRotate, Size(850,850), 0,0, INTER_CUBIC);
			transpose(matRotate, matMainMonitor);							//cameras in the robot are rotated
			flip(matMainMonitor, matMainMonitor, 0);

			/* Show in screen */
			imshow("inMoov", composition);
			imshow("main monitor", matMainMonitor);	
		} else {
			composition = Mat::zeros(200, 600, frameL.type());
			putText(composition, "Camaras desactivadas.", Point(0,50), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255,255,255), 1);
			putText(composition, "Presione ESC para detener el programa.", Point(0,100), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255,255,255), 1);

			imshow("main monitor", composition);
		}

		/* End when ESC is pressed */
		if (waitKey(delay) == 27) {
			std::cout << "Tecla ESC presionada. El programa finalizara." << std::endl;
			getline(cin,s);
			return 1; 
		}
	}
}

int End () {
	controller.removeListener(listener);

	return 0;
}

int main (int argc, const char* argv[]) {

	String disable;
	cout << "Ingrese funciones a desactivar [camera, oculus, leap, arduino]. ENTER para continuar." << endl;
	
	getline(cin,disable);

	if (disable.find("camera") != string::npos) {
		cameraEnabled = false;
		cout << "Camera disabled" << endl;
	}
	if (disable.find("oculus") != string::npos) {
		oculusEnabled = false;
		cout << "Oculus disabled" << endl;
	}
	if (disable.find("arduino") != string::npos) {
		arduinoEnabled = false;
		cout << "Arduino disabled" << endl;
	}
	if (disable.find("leap") != string::npos) {
		leapEnabled = false;
		cout << "Leap disabled" << endl;
	}

	//cameraEnabled = false;
	//leapEnabled = false;
	//arduinoEnabled = false;

	if (!cameraEnabled && !oculusEnabled && !arduinoEnabled && !leapEnabled) {
		cout << "\nTodas las funciones desactivadas. El programa terminara." << endl;
		getline(cin,disable);
	} else {
		if (Init() > 0) {
			Loop();
			End();
		}
	}
}
