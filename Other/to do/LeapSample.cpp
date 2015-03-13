/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <string.h>
#include "Leap.h"
#include <thread> //Added, for sleep
#include <chrono>
#include <math.h>
#include "Serial.cpp"
using namespace Leap;
Serial* SP;

class SampleListener : public Listener {
  public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);

  private:
};

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

void SampleListener::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
  std::cout << "Connected" << std::endl;
  /*
  controller.enableGesture(Gesture::TYPE_CIRCLE);
  controller.enableGesture(Gesture::TYPE_KEY_TAP);
  controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
  controller.enableGesture(Gesture::TYPE_SWIPE);
  */
}

void SampleListener::onDisconnect(const Controller& controller) {
  // Note: not dispatched when running in a debugger.
  std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
  // Get the most recent frame and report some basic information
  std::this_thread::sleep_for(std::chrono::milliseconds(30)); //Added Sleep
  const Frame frame = controller.frame();
  std::cout //<< "Frame id: " << frame.id()
            //<< ", timestamp: " << frame.timestamp()
            //<< ", hands: " << frame.hands().count()
			<< "Hands: " << frame.hands().count()
			<< ", Extended fingers: " << frame.fingers().extended().count()
			<< "\n"
            //<< ", extended fingers: " << frame.fingers().extended().count()
            //<< ", tools: " << frame.tools().count()
            //<< ", gestures: " << frame.gestures().count() 
			<< std::endl;

  HandList hands = frame.hands();
  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    // Get the first hand
    const Hand hand = *hl;
    std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
	
    std::cout << std::string(1, ' ') << handType 
		// << ", id: " << hand.id()
        //      << ", Palm position: " << hand.palmPosition() <<"\n" 
		<< std::endl;
	
    // Get the hand's normal vector and direction
    const Vector normal = hand.palmNormal();
    const Vector direction = hand.direction();
	
    // Calculate the hand's pitch, roll, and yaw angles
	//std::cout << std::string(1, ' ') << direction << std::endl;
	/*
    std::cout << std::string(2, ' ') <<  "Pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
              << "Roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
              << "Yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" <<"\n" << std::endl;
	*/
    // Get the Arm bone
	
    Arm arm = hand.arm();
	/*
    std::cout << std::string(2, ' ') 
			  <<  "Arm direction: " << arm.direction()
              << " Wrist position: " << arm.wristPosition() << "\n"
              << " Elbow position: " << arm.elbowPosition() <<"\n" 
			  << " Angle: " << std::endl;
	*/	  
    // Get fingers
	
    const FingerList fingers = hand.fingers();
    
	
	for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      char c;
	const Finger finger = *fl;



	  
      std::cout << std::string(1, ' ') <<  fingerNames[finger.type()]
                //<< " finger, id: " << finger.id()
                //<< ", length: " << finger.length()
                //<< "mm, width: " << finger.width() 
				<< std::endl;
				
      
	  // Get finger bones
	
	  Vector vectors[3];	
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
		

		//angle1 = vectors[0].angleTo(vectors[1])* 180/PI*2.45;
		//angle2 = vectors[1].angleTo(vectors[2])* 180/PI*1.75;
		angle1 = vectors[0].angleTo(vectors[1])* 180/PI;
		angle2 = vectors[1].angleTo(vectors[2])* 180/PI;
		angle = (angle1 + angle2)*2;
	  }else{
		for (int b = 0; b < 3; ++b) {
			Bone::Type boneType = static_cast<Bone::Type>(b);
	    //Bone::Type boneType = static_cast<Bone::Type>(1);
			Bone bone = finger.bone(boneType);
		
			vectors[b] = bone.direction();
		
        //std::cout << std::string(6, ' ') <<  boneNames[boneType]
                  //<< " bone, start: " << bone.prevJoint()
                  //<< "end: " << bone.nextJoint()
          //        << "direction: " << vectors[b] <<"\n" << std::endl;
			
		}
		angle1 = vectors[0].angleTo(vectors[1])* 180/PI;
		angle2 = vectors[1].angleTo(vectors[2])* 180/PI;
		angle = (angle1 + angle2)*1.085;
	  }	

	  if(angle > 180){
		angle = 180;
	  }
		std::cout << std::string(6, ' ') <<"Angulo1: "<<angle1 <<" Angulo2: "<< angle2 <<" Angulo completo: " << angle << std::endl;
		std::cout << std::string(2, ' ') <<"Angulo completo: " << angle << std::endl;
		
		c = (char)(angle);
		SP ->WriteData(&c,1);
    }
	
  }

  // Get tools
  /*
  const ToolList tools = frame.tools();
  for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
    const Tool tool = *tl;
    std::cout << std::string(2, ' ') <<  "Tool, id: " << tool.id()
              << ", position: " << tool.tipPosition()
              << ", direction: " << tool.direction() << std::endl;
  }
  */
  // Get gestures
  /*
  const GestureList gestures = frame.gestures();
  for (int g = 0; g < gestures.count(); ++g) {
    Gesture gesture = gestures[g];

    switch (gesture.type()) {
      case Gesture::TYPE_CIRCLE:
      {
        CircleGesture circle = gesture;
        std::string clockwiseness;

        if (circle.pointable().direction().angleTo(circle.normal()) <= PI/2) {
          clockwiseness = "clockwise";
        } else {
          clockwiseness = "counterclockwise";
        }

        // Calculate angle swept since last frame
        float sweptAngle = 0;
        if (circle.state() != Gesture::STATE_START) {
          CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
          sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
        }
        std::cout << std::string(2, ' ')
                  << "Circle id: " << gesture.id()
                  << ", state: " << stateNames[gesture.state()]
                  << ", progress: " << circle.progress()
                  << ", radius: " << circle.radius()
                  << ", angle " << sweptAngle * RAD_TO_DEG
                  <<  ", " << clockwiseness << std::endl;
        break;
      }
      case Gesture::TYPE_SWIPE:
      {
        SwipeGesture swipe = gesture;
        std::cout << std::string(2, ' ')
          << "Swipe id: " << gesture.id()
          << ", state: " << stateNames[gesture.state()]
          << ", direction: " << swipe.direction()
          << ", speed: " << swipe.speed() << std::endl;
        break;
      }
      case Gesture::TYPE_KEY_TAP:
      {
        KeyTapGesture tap = gesture;
        std::cout << std::string(2, ' ')
          << "Key Tap id: " << gesture.id()
          << ", state: " << stateNames[gesture.state()]
          << ", position: " << tap.position()
          << ", direction: " << tap.direction()<< std::endl;
        break;
      }
      case Gesture::TYPE_SCREEN_TAP:
      {
        ScreenTapGesture screentap = gesture;
        std::cout << std::string(2, ' ')
          << "Screen Tap id: " << gesture.id()
          << ", state: " << stateNames[gesture.state()]
          << ", position: " << screentap.position()
          << ", direction: " << screentap.direction()<< std::endl;
        break;
      }
      default:
        std::cout << std::string(2, ' ')  << "Unknown gesture type." << std::endl;
        break;
    }
  }
  */
  //if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
  if (!frame.hands().isEmpty()) {
    std::cout << std::endl;
  }
}

void SampleListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
  std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
  std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
  }
}

void SampleListener::onServiceConnect(const Controller& controller) {
  std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
  std::cout << "Service Disconnected" << std::endl;
}

int main(int argc, char** argv) {

 
  SP = new Serial("\\\\.\\COM38");
  if(SP->IsConnected() == false) {
	return 0;
  }
 

  // Create a sample listener and controller
  SampleListener listener;
  Controller controller;

  // Have the sample listener receive events from the controller
  controller.addListener(listener);

  if (argc > 1 && strcmp(argv[1], "--bg") == 0)
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

  // Keep this process running until Enter is pressed
  std::cout << "Press Enter to quit..." << std::endl;
  std::cin.get();

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
