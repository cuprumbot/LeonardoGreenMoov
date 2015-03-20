using namespace Leap;

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
}

void SampleListener::onDisconnect(const Controller& controller) {
  std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
  // Get the most recent frame and report some basic information
  std::this_thread::sleep_for(std::chrono::milliseconds(30)); //Added Sleep
  const Frame frame = controller.frame();
  std::cout << "Hands: " << frame.hands().count()
            << ", Extended fingers: " << frame.fingers().extended().count()
            << "\n"
		        << std::endl;

  HandList hands = frame.hands();

  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    
    // Get the first hand
    const Hand hand = *hl;
    std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
	
    std::cout << std::string(1, ' ') 
              << handType 
		          << std::endl;
	
    // Get the hand's normal vector and direction
    const Vector normal = hand.palmNormal();
    const Vector direction = hand.direction();
	

    // Get the Arm bone
    Arm arm = hand.arm();

    // Get fingers
    const FingerList fingers = hand.fingers();
    
	for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      char c;
	const Finger finger = *fl;

      std::cout << std::string(1, ' ') 
                <<  fingerNames[finger.type()]
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
		
		angle1 = vectors[0].angleTo(vectors[1])* 180/PI;
		angle2 = vectors[1].angleTo(vectors[2])* 180/PI;
		angle = (angle1 + angle2)*2;
	  }else{
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
		std::cout << std::string(6, ' ') <<"Angulo1: "<<angle1 <<" Angulo2: "<< angle2 <<" Angulo completo: " << angle << std::endl;
		std::cout << std::string(2, ' ') <<"Angulo completo: " << angle << std::endl;
		
		c = (char)(angle);
		SP ->WriteData(&c,1);
    }
	
  }

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
