#include "Leap.h"
#include <windows.h>
#include <string.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

using namespace std;
using namespace Leap;

class SampleListener : public Listener {
public:
	string swipeDirection = "";
	//HDC const dc = GetDC(0);
	//HRGN circle = CreateEllipticRgn(0, 0, 10, 10);
	HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));

	bool tabbing = false;

	Leap::Hand manodx = Leap::Hand::invalid();
	Leap::Hand manosx = Leap::Hand::invalid();

	float rr = 100.0;
	float minDiff = 20.0;
	float smoothFactor = 2.0;

	float prevX = 0.0;
	float prevY = 0.0;
	float prevZ = 0.0;

	bool enableLClick = true;
	bool enableRClick = true;

	bool enablePinch = true;
	bool pinching = false;

	bool enableGrab = true;
	bool grabbing = false;
	bool playpause = false;

	Leap::SwipeGesture swipeG = Leap::Gesture::invalid();
	Leap::CircleGesture circleG = Leap::Gesture::invalid();
	Leap::KeyTapGesture keytapG = Leap::Gesture::invalid();
	
	INPUT buffer[1];

	//FRAME EVENTS
	virtual void onConnect(const Controller&);
	virtual void onFrame(const Controller&);

	//MOUSE EVENTS
	virtual void mouseMove(int x, int y);
	virtual void leftClick();
	virtual void leftGrab();
	virtual void leftDrop();

	virtual void rightClick();

	virtual void wheelClickDown();
	virtual void wheelClickUp();
	virtual void mouseScroll(float y);
	
	virtual void pressKey(WORD vkey);
	virtual void downKey(WORD vkey, INPUT *buffer);
	virtual void upKey(INPUT *buffer);

	//UTILITY FUNCTIONS
	virtual float distance(Leap::Vector v1, Leap::Vector v2);
};

void SampleListener::onConnect(const Controller& controller) {

	bool backgroundModeAllowed = controller.config().getInt32("background_app_mode") == 2;
	if (!backgroundModeAllowed) {
		controller.config().setInt32("background_app_mode", 2);
		controller.config().save();
	}
	controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
	
	cout << "Connected" << endl;

	//controller.enableGesture(Gesture::TYPE_SWIPE);
	//controller.enableGesture(Gesture::TYPE_KEY_TAP);
	//controller.enableGesture(Gesture::TYPE_CIRCLE);
	
	//keytap settings
	//controller.config().setFloat("Gesture.KeyTap.MinDownVelocity", 10.0);

	//circle settings
	//...

	//swipe settings
	//...

	controller.config().save();
}

void SampleListener::onFrame(const Controller& controller) {
	
	const Frame frame = controller.frame();
	//cout << "Frame available: " << frame.id() << endl;
	
	if (frame.hands().rightmost().isRight())
		manodx = frame.hands().rightmost();
	if (frame.hands().leftmost().isLeft())
		manosx = frame.hands().leftmost();

	//map rounded coordinates to screen coordinates
	//clip coordinates in determined movement range - interaction box

	Vector nPos;
	float nx, nsx, ny, nsy;
	if (manodx.isValid()) {
		Vector nsPos = frame.interactionBox().normalizePoint(manodx.stabilizedPalmPosition());
		//Vector nsPosalt = frame.interactionBox().normalizePoint(Leap::Vector(manodx.stabilizedPalmPosition().x, manodx.stabilizedPalmPosition().y - 50.0, manodx.stabilizedPalmPosition().z));
		nsx = nsPos.x * SCREEN_WIDTH;
		nsy = (1 - nsPos.y) * SCREEN_HEIGHT;

		nPos = frame.interactionBox().normalizePoint(manodx.palmPosition());
		//Vector nPosalt = frame.interactionBox().normalizePoint(Leap::Vector(manodx.palmPosition().x, manodx.palmPosition().y - 50.0, manodx.palmPosition().z));
		nx = nPos.x * SCREEN_WIDTH;
		ny = (1 - nPos.y) * SCREEN_HEIGHT;
	}

	//for every finger, check if it's extended
	//to check different gestures being performed
	int thuext = manodx.fingers()[0].isExtended();
	int indext = manodx.fingers()[1].isExtended();
	int midext = manodx.fingers()[2].isExtended();
	int rinext = manodx.fingers()[3].isExtended();
	int pinext = manodx.fingers()[4].isExtended();

	//total number of extended fingers, excluding thumb because special case
	int countExtended = indext + midext + rinext + pinext;

	//CURSOR MOVEMENT mapped to palmPosition
	if (manodx.isValid()) {
		//if (manodx.isValid() && !stopHand) {
		if (manodx.palmNormal().angleTo(Leap::Vector::down()) < 1.5) {
			//move cursor
			//if left clicking
			if ((countExtended == 1 && indext)
				//if right clickng
				|| (countExtended == 2 && indext && midext)
				//if pinching or grabbing
				|| pinching || grabbing
				|| (!indext && midext && rinext && pinext)) {

				//normalized stabilized position for smooth movement and flickering tolerance
				//SetCursorPos(nsx, nsy);

			}
			else {
				//normalized position for smooth but responsive movement
				//SetCursorPos(nx, ny);
				
			}
		}
	}

	//for every finger, check tip coordinates to compute distances
	/*Leap::Vector thumbTip = manodx.fingers()[0].tipPosition();
	Leap::Vector indexTip = manodx.fingers()[1].tipPosition();
	Leap::Vector middleTip = manodx.fingers()[2].tipPosition();
	Leap::Vector ringTip = manodx.fingers()[3].tipPosition();
	Leap::Vector pinkyTip = manodx.fingers()[4].tipPosition();

	//three major distances to determine closed or open hand
	//thumb special case not considered
	float IMdist = distance(indexTip, middleTip);
	float MRdist = distance(middleTip, ringTip);
	float RPdist = distance(ringTip, pinkyTip);*/

	/*bool stopHand = IMdist < 22 && MRdist < 22 && RPdist < 28 && countExtended==4;
	if (stopHand) {
		cout << "STOP" << endl;
	}*/

	//Leap::Pointable previndex = controller.frame(1).hands().rightmost().pointable(manodx.fingers()[1].id());
	Leap::Pointable clicker = manodx.pointable(manodx.fingers()[1].id());
	
	if (indext && countExtended==1 
		&& enableLClick
		&& clicker.touchZone() == 2 && clicker.touchDistance() < -0.4) {

		//leftClick();
		//cout << clicker.touchZone() << " " << clicker.touchDistance() << endl;
		cout << "--- LEFT CLICK ---" << endl;
		enableLClick = false;
	}
	else if(clicker.touchZone() != 2 && clicker.touchDistance() > -0.4 && !enableLClick) {
		enableLClick = true;
	}

	if (indext && midext && countExtended == 2 
		&& enableRClick
		&& clicker.touchZone() == 2 && clicker.touchDistance() < -0.4) {

		//rightClick();
		//cout << clicker.touchZone() << " " << clicker.touchDistance() << endl;
		cout << "--- RIGHT CLICK ---" << endl;
		enableRClick = false;
	}
	else if (clicker.touchZone() != 2 && clicker.touchDistance() > -0.4 && !enableRClick) {
		enableRClick = true;
	}
	
	if (countExtended != 3 && tabbing) {
		tabbing = false;
		Sleep(50);
		upKey(buffer);
	}

	Leap::Vector vec;
	Leap::GestureList gestures = frame.gestures();
	HWND foregroundWindow = GetForegroundWindow();
	//Leap::Gesture prevG = Leap::Gesture::invalid();

	//int *gdirs = new int(gestures.count());
	//int *gids = new int(gestures.count());
	for (Leap::GestureList::const_iterator gl = gestures.begin(); gl != gestures.end(); gl++) {
		
		switch ((*gl).type()) {
			case 1:

				swipeG = Leap::SwipeGesture(*gl);
				vec = swipeG.direction();

				if (swipeG.pointable().id() == manodx.fingers()[1].id()) {
					cout << "swipe indice" << endl;

					if (swipeG.state() == 2) {
						float angle = vec.angleTo(Leap::Vector::right());
						if (angle < 0.6) {
							swipeDirection = "right";
						}
						else if (angle > 2.2) {
							swipeDirection = "left";
						}
						else if (angle > 1.0 && angle < 1.8) {
							angle = vec.angleTo(Leap::Vector::up());

							if (angle < 0.6) {
								swipeDirection = "up";
							}
							else if (angle > 2.2) {
								swipeDirection = "down";
							}
						}
					}

					else if (swipeG.state() == 3) {
						if (swipeDirection == "right") {
							switch (countExtended) {
							case 1:
								pressKey(VK_RIGHT);
								break;
							case 4:
								pressKey(VK_MEDIA_NEXT_TRACK);
								break;
							case 2:
								pressKey(VK_BROWSER_FORWARD);
								break;
							case 3:
								//TODO: CLICK WITH 3 FINGERS IF TABBING ACTIVE
								if (!tabbing) {
									tabbing = true;
									downKey(VK_MENU, buffer);
									Sleep(50);
									pressKey(VK_TAB);
								}
								else if (tabbing) {
									pressKey(VK_TAB);
								}
								break;
							default:
								break;
							}
						}
						else if (swipeDirection == "left") {
							switch (countExtended) {
							case 1:
								pressKey(VK_LEFT);
								break;
							case 4:
								pressKey(VK_MEDIA_PREV_TRACK);
								break;
							case 2:
								pressKey(VK_BROWSER_BACK);
								break;
							case 3:
								//TODO: CLICK WITH 3 FINGERS IF TABBING ACTIVE
								INPUT bufferShift[1];
								if (!tabbing) {
									tabbing = true;
									downKey(VK_MENU, buffer);
									Sleep(30);
									downKey(VK_SHIFT, bufferShift);
									Sleep(30);
									pressKey(VK_TAB);
									Sleep(30);
									upKey(bufferShift);
								}
								else if (tabbing) {
									downKey(VK_SHIFT, bufferShift);
									Sleep(30);
									pressKey(VK_TAB);
									Sleep(30);
									upKey(bufferShift);
								}
								break;
							default:
								break;
							}
						}
						else if (swipeDirection == "down") {
							switch (countExtended) {
							case 4:
								ShowWindow(foregroundWindow, SW_MINIMIZE);
								break;
							case 2:
								pressKey(VK_BROWSER_REFRESH);
								break;
							case 3:
								pressKey(VK_VOLUME_MUTE);
								break;
							default:
								break;
							}
						}
						else if (swipeDirection == "up") {
							switch (countExtended) {
							case 4:
								pressKey(VK_LWIN);
								break;
							case 2:
								pressKey(VK_BROWSER_HOME);
								break;
							case 3:
								pressKey(VK_F11);
								break;
							default:
								break;
							}
						}
					}
				}

				//when swipe starts, check direction
				/*if (swipeG.state() == 1) {
					int axis = max(abs(swipeG.direction().x), abs(swipeG.direction().y));
					//if (swipeG.direction().x < 0)
				}

				//while swipe is going, update the force
				if (swipeG.state() == 2) {
					
				}

				//when swipe ends, perform action
				if (swipeG.state() == 3) {

				}*/
				/*
				if (countExtended >= 4) {
						
					if (vec.y < -0.8) {
						HWND foregroundWindow = GetForegroundWindow();
						ShowWindow(foregroundWindow, SW_MINIMIZE);
					}
					else if (vec.x > 0.9) {
						pressFWKey(&Input);
					}
					else if (vec.x < -0.8) {
						pressBWKey(&Input);
					}
					*/

				cout << "SWIPE ID "<< swipeG.id() << "\tstate: " << (*gl).state() << "\tdirection: " << vec.angleTo(Leap::Vector::right()) << endl;
				
				//}
				break;

			case 4:
				circleG = Leap::CircleGesture(*gl);
				if ((countExtended + thuext) == 5) {
					if (circleG.pointable().direction().angleTo(circleG.normal()) <= Leap::PI / 2)
						cout << "CIRCLE clockwise" << endl;
					else
						cout << "CIRCLE counter clockwise" << endl;
				}
				break;

			default:
				//cout << "! gesture not recognized !" << endl;
				break;
		}
	}

	//grab inside a page to scroll
	if (manodx.isValid() && enableGrab) {
		if (manodx.grabAngle() >= 1.00) {// && !grabbing && countExtended == 0) {
			cout << "GRAB IN" << endl;
			grabbing = true;
			//wheelClickDown();
		}
		else if (manodx.grabAngle() < 1.00) {// && grabbing) {
			cout << "GRAB OUT" << endl;
			grabbing = false;
			//wheelClickUp();
		}
	}

	//use pinch to drag and drop
	/*if (manodx.isValid() && enablePinch) {
		//if (!indext && countExtended >= 1 && manodx.pinchDistance() <= 50.00) && !pinching) {
		if(manodx.pinchDistance() <= 60.00) {
			//cout << thuext << " " << indext << " " << midext << " " << rinext << " " << pinext << endl;
			cout << "PINCH IN" << endl;
			pinching = true;
			//leftGrab();
		}
		else if (manodx.pinchDistance() > 60.00 ) { //&& pinching) {
			pinching = false;
			cout << "PINCH OUT" << endl;
			//leftDrop();
		}
	}*/
	

	cout << "--- --- ---" << endl;
	cout << "grabAngle: " << manodx.grabAngle() << endl;
	cout << thuext << " " << indext << " " << midext << " " << rinext << " " << pinext << endl;

	//Rock and Roll gesture
	bool RnR = indext && !midext && !rinext && pinext;
	bool facingScreen = manodx.palmNormal().angleTo(Leap::Vector::forward()) < 0.7
					|| manodx.palmNormal().angleTo(Leap::Vector::forward()) > 2.5;

	//rock and roll to play and pause media
	/*if (!playpause) {
		if (RnR && facingScreen) {
			playpause = true;
			pressKey(VK_MEDIA_PLAY_PAUSE);
			cout << "PLAY" << endl;
		}
	}
	else {
		if (!RnR || !facingScreen) {
			playpause = false;
		}
	}*/
	
}

void SampleListener::leftClick() {
	INPUT Input = {};

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));

	ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));

	//Sleep(500);
}

void SampleListener::leftGrab() {
	INPUT Input = {};

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));
}

void SampleListener::leftDrop() {
	INPUT    Input = {};

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void SampleListener::rightClick() {
	INPUT Input = {};

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));

	ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void SampleListener::wheelClickDown() {
	INPUT    Input = {};

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
	::SendInput(1, &Input, sizeof(INPUT));
}

void SampleListener::wheelClickUp() {
	INPUT    Input = {};

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void SampleListener::mouseScroll(float y) {
	HWND foregroundWindow;
	foregroundWindow = GetForegroundWindow();
	mouse_event(MOUSEEVENTF_WHEEL, 0, 0, y, 0);
}

void SampleListener::mouseMove(int x, int y) {
	INPUT Input = {};

	Input.type = INPUT_MOUSE;
	Input.mi.dx = x; //(x * (0xFFFF / SCREEN_WIDTH));
	Input.mi.dy = y; //(y * (0xFFFF / SCREEN_HEIGHT));
	Input.mi.mouseData = 0;
	Input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);
	Input.mi.time = 0;
	Input.mi.dwExtraInfo = 0;

	::SendInput(1, &Input, sizeof(INPUT));
}

void SampleListener::pressKey(WORD vkey) {
	INPUT Input = {};

	Input.type = INPUT_KEYBOARD;
	Input.ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
	Input.ki.time = 0;
	Input.ki.dwExtraInfo = 0;
	Input.ki.wVk = vkey;
	Input.ki.dwFlags = 0; // there is no KEYEVENTF_KEYDOWN
	SendInput(1, &Input, sizeof(INPUT));

	Input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &Input, sizeof(INPUT));
}

void SampleListener::downKey(WORD vkey, INPUT *buffer) {

	buffer->type = INPUT_KEYBOARD;
	buffer->ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
	buffer->ki.time = 0;
	buffer->ki.dwExtraInfo = 0;
	buffer->ki.wVk = vkey;
	buffer->ki.dwFlags = 0; // there is no KEYEVENTF_KEYDOWN
	SendInput(1, buffer, sizeof(INPUT));
}

void SampleListener::upKey(INPUT *buffer) {

	buffer->ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, buffer, sizeof(INPUT));
}

/*
void mouseSetup(INPUT *&Input)
{
	Input.type = INPUT_MOUSE;
	Input.mi.dx = (0 * (0xFFFF / SCREEN_WIDTH));
	Input.mi.dy = (0 * (0xFFFF / SCREEN_HEIGHT));
	Input.mi.mouseData = 0;
	Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
	Input.mi.time = 0;
	Input.mi.dwExtraInfo = 0;
}
*/

float SampleListener::distance(Leap::Vector v1, Leap::Vector v2) {
	return sqrt(pow((v1[0] - v2[0]), 2.0) + pow((v1[2] - v2[2]), 2.0));
}

