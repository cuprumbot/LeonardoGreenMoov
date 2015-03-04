#include <iostream>
//#include <string>

/* VR */
#include "OVR.h"

/* CV */
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

/* Serial */
#include "Serial.cpp"

using namespace cv;
using namespace std;

/* VR */
ovrHmd hmd;				//handler for the Oculus

/* CV */
VideoCapture capL(0);	//capture cameras
VideoCapture capR(1);
int capWidth   = 1080;	//size for capturing
int capHeight  = 720;
int showWidth  = 1000;	//size for showing
int showHeight = 960;
double realWidthL = 0;
double realHeightL = 0;
double realWidthR = 0;
double realHeightR = 0;

/* Serial */
Serial* SP;

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
	/*
	SP = new Serial("\\\\.\\COM38");
	if (SP->IsConnected() == false) {
		return -2;
	} else {
		cout << "Serial OK!" << endl;
	}
	*/

	/* Success */
	return 1;
}

int Loop () {
	Mat frameL, frameR;
	Mat composition;

	int frame = 0;
	char deg = 0;

	while (true) {
		/* Get orientation as a Quaternion */
		ovrHmd_BeginFrameTiming(hmd, frame);
		ovrTrackingState trackState = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
		ovrQuatf curOrient = trackState.HeadPose.ThePose.Orientation;
		//cout << curOrient.x << " " << curOrient.y << " " << curOrient.z << " " << curOrient.w << endl;
		ovrHmd_EndFrameTiming(hmd);
		frame++;

		/* Convert to Euler angles */
		float eyeYaw, eyePitch, eyeRoll;
		OVR::Quatf l_Orientation = OVR::Quatf(curOrient);
		l_Orientation.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&eyeYaw, &eyePitch, &eyeRoll);
		cout << OVR::RadToDegree(eyeYaw) << "   \t" << OVR::RadToDegree(eyePitch) << "   \t" << OVR::RadToDegree(eyeRoll) << endl;
		
		/* Write to serial */
		deg = (char)(eyeYaw * 2);
		deg = (deg < 0) ? 0 : deg;
		deg = (deg > 255) ? 255 : deg;
		//deg = 255;
		//SP->WriteData(&deg, 1);

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
		composition = Mat(1920, 1080, frameL.type());				//create matrix to join the captured images, args: rows, columns
		Mat left  (composition, Rect(0,   0, showWidth, showHeight));	//put frames on matrix, args: x, y, w, h
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

		/* Release */
		left.release();
		right.release();
		matRotate.release();
		matMainMonitor.release();
		frameL.release();
		frameR.release();
		composition.release();

		/* End when ESC is pressed */
		if (waitKey(13) == 27) {
			std::cout << "ESC key is pressed by user" << std::endl;
			return 1; 
		}
	}
}

int End () {
	return 0;
}

int main (int argc, const char* argv[]) {
	if (Init() > 0) {
		Loop();
		End();
	}
}