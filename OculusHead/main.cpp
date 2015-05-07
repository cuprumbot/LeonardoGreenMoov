#include <iostream>
#include <string.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <string.h>
#include <thread>
#include <chrono>
#include <math.h>

/* ola ke ase */

/* VR */
#include "OVR.h"

/* CV */
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

/* Serial */
#include "Serial.cpp"

/* Leap */
#include "Leap.h"

using namespace Leap;
using namespace cv;
using namespace std;

/* VR */
ovrHmd hmd;					//handler for the Oculus

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
int originalAngles[] = {0,0,0,0,0};
char fingerAngles[] = {180,180,180,180,180};

/*
	CLASE LeapListener
	Contiene los listeners necesarios para las acciones aceptadas por el Leap Motion
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
	/*
	std::cout << "Hands: " << frame.hands().count()
			<< ", Extended fingers: " << frame.fingers().extended().count()
			<< "\n"
			<< std::endl;
	*/

	HandList hands = frame.hands();

	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		
		// Get the first hand
		const Hand hand = *hl;
		std::string handType = hand.isLeft() ? "Left hand" : "Right hand";

		/*
		std::cout << std::string(1, ' ') 
				<< handType 
				<< std::endl;
		*/

		// Get the hand's normal vector and direction
		const Leap::Vector normal = hand.palmNormal();
		const Leap::Vector direction = hand.direction();
	
		// Get the Arm bone
		Arm arm = hand.arm();

		// Get fingers
		const FingerList fingers = hand.fingers();

		int counter = 0;
			
		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			char c;
			const Finger finger = *fl;

			/*
			std::cout << std::string(1, ' ') 
					<<  fingerNames[finger.type()]
					<< std::endl;
			*/

			// Get finger bones
			Leap::Vector vectors[3];	
			int angle, angle1, angle2;

			if(fl == fingers.begin()){
				Bone::Type boneType = static_cast<Bone::Type>(1);
				Bone bone = finger.bone(boneType);
				vectors[0] = bone.direction();
				
				boneType = static_cast<Bone::Type>(2);
				bone = finger.bone(boneType);
				vectors[1] = bone.direction();
				
				boneType = static_cast<Bone::Type>(3);
				bone = finger.bone(boneType);
				vectors[2] = bone.direction();
				
				angle1 = vectors[0].angleTo(vectors[1])* 180/PI;
				angle2 = vectors[1].angleTo(vectors[2])* 180/PI;
				//angle = (angle1 + angle2)*2;
				
				//cout << "a1: " << (int)angle1 << "\ta2: " << (int)angle2 <<endl;

				//TO DO: caso especial del pulgar
				if (angle1 > 35) angle = 180;
				else if (angle2 > 40) angle = 180;
				else angle = 0;

				//if (angle == 180) cout << "CERRADO" << endl;
				//else cout << "ABIERTO" << endl;

			} else {
				for (int b = 0; b < 3; ++b) {
					Bone::Type boneType = static_cast<Bone::Type>(b);
					Bone bone = finger.bone(boneType);
				
					vectors[b] = bone.direction();		
				}
				angle1 = vectors[0].angleTo(vectors[1])* 180/PI;
				angle2 = vectors[1].angleTo(vectors[2])* 180/PI;
				angle = (angle1 + angle2)*1.085;
			}	

			if(angle > 180){
				angle = 180;
			}
			
			/*
			std::cout << std::string(6, ' ') << "Angulo1: "<<angle1 <<" Angulo2: "<< angle2 <<" Angulo completo: " << angle << std::endl;
			std::cout << std::string(2, ' ') << "Angulo completo: " << angle << std::endl;
			*/

			originalAngles[counter] = angle;

			c = (char)(angle);
			fingerAngles[counter] = c;

			counter++;
		} //for end: FingerList iterator
	
	} //for end: HandList iterator

	if (!frame.hands().isEmpty()) {
		//std::cout << std::endl;
	}
}
/*
	FIN DE LA CLASE LeapListener
*/

/* Leap */
LeapListener listener;
Controller	 controller;

int Init () {
	/* Initialize and configure the Oculus */
	ovr_Initialize();
	hmd = ovrHmd_Create(0);
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);	// Tracking configuration	

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

	cout << "Frame size 1: " << realWidthL << " x " << realHeightL << endl;
	cout << "Frame size 2: " << realWidthR << " x " << realHeightR << endl;

	/* Windows for both cameras */
	namedWindow("inMoov",CV_WINDOW_NORMAL);
	cvSetWindowProperty( "inMoov", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN );
	cvResizeWindow("inMoov", 1080, 1920);
	cvMoveWindow("inMoov", 1601, 0);

	/* Window for main monitor */
	namedWindow("main monitor",CV_WINDOW_AUTOSIZE);
	cvMoveWindow("main monitor", 0, 0);

	/* Connect to serial */
	SP = new Serial("\\\\.\\COM43");	//TO DO: pedir numero de puerto en consola

	//comentario

	if (SP->IsConnected() == false) {
		return -2;
	} else {
		cout << "Serial OK!" << endl;
	}

	controller.addListener(listener);

	/* Success */
	return 1;
}

int Loop () {
	Mat frameL, frameR;
	Mat composition;

	int delay = 1000/fps;
	int frame = 0;
	int counter = 10;

	char cYaw;
	char cPitch;
	//char cRoll;

	while (true) {
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
		//cout << eyeYaw << "   \t" << eyePitch << "   \t" << eyeRoll << endl;
		
		/* Write to serial */
		//if (counter++ > 3) {

			/* Pitch */
			cPitch	= (char)(eyePitch + 90);

			/* Yaw */
			/* Gimbal lock on pitches 0 and 180, don't change yaw*/
			if ((eyePitch+90) > 10 && (eyePitch+90) < 170) {
				if (eyeYaw >= 0 && eyeYaw <= 180) {
					cYaw = (char)eyeYaw;
				} else if (eyeYaw > -90) {
					cYaw = (char)0;
				} else if (eyeYaw < -90) {
					cYaw = (char)180;
				}
			} else if ((eyePitch+90) >= 170) {
				cPitch = (char)180;
			} else {
				cPitch = (char)0;
			}

			/* Roll */
			//unused

			SP->WriteData(&cYaw, 1);
			SP->WriteData(&cPitch, 1);
			
			char c;
			for (int arrIndex = 0; arrIndex < 5; arrIndex++) {
				//if (arrIndex == 0) {
				//	fingerAngles[arrIndex] = (originalAngles[0] > 50 && originalAngles[0] < 110) ? 0 : 180;	
				//} else {
					fingerAngles[arrIndex] = (originalAngles[arrIndex] < 70) ? 0 : 180;
				//}
				c = fingerAngles[arrIndex];
				SP->WriteData(&c, 1);
			}

			/*
			cout << "Yaw: " << (int)cYaw 
				<< "\tPitch: " << (int)cPitch 
				<< "\tFingers: " 
					<< (int)originalAngles[0] << "\t"
					<< (int)originalAngles[1] << "\t"
					<< (int)originalAngles[2] << "\t"
					<< (int)originalAngles[3] << "\t"
					<< (int)originalAngles[4] << "\t"
				<< endl;
			*/

			
			cout << "Yaw: " << (int)cYaw 
				<< "\tPitch: " << (int)cPitch 
				<< "\tFingers: " 
					<< (int)fingerAngles[0] << "\t"
					<< (int)fingerAngles[1] << "\t"
					<< (int)fingerAngles[2] << "\t"
					<< (int)fingerAngles[3] << "\t"
					<< (int)fingerAngles[4] << "\t"
				<< endl;
			
			//counter = 1;
		//}

		/* Read from cameras */
		bool bSuccessL = capL.read(frameL);
		bool bSuccessR = capR.read(frameR);

		if (!bSuccessL || !bSuccessR) {
			cout << "Failed to read from camera" << endl;
			return -1;
		}

		/* Resize frames */
		resize(frameL, frameL, Size(showWidth,showHeight), 0, 0, INTER_CUBIC);
		resize(frameR, frameR, Size(showWidth,showHeight), 0, 0, INTER_CUBIC);
		
		/* Make composite image */
		composition = Mat(1920, 1080, frameL.type());					//create matrix to join the captured images, args: rows, columns
		Mat left  (composition, Rect(0, 0, showWidth, showHeight));		//put frames on matrix, args: x, y, w, h
		frameL.copyTo (left);
		Mat right (composition, Rect(1080-showWidth, 960, showWidth, showHeight));
		frameR.copyTo (right);

		/* Make image for main monitor */
		Mat matRotate		(500,500,frameL.type());
		Mat matMainMonitor	(500,500,frameL.type());
		resize(frameR, matRotate, Size(500,500), 0,0, INTER_CUBIC);
		transpose(matRotate, matMainMonitor);							//cameras in the robot are rotated
		flip(matMainMonitor, matMainMonitor, 0);

		/* Show in screen */
        imshow("inMoov", composition);
		imshow("main monitor", matMainMonitor);	

		/* End when ESC is pressed */
		if (waitKey(delay) == 27) {
			std::cout << "ESC key is pressed by user" << std::endl;
			return 1; 
		}
	}
}

int End () {
	controller.removeListener(listener);

	return 0;
}

int main (int argc, const char* argv[]) {
	if (Init() > 0) {
		Loop();
		End();
	}
}
