#pragma once
#include <input/input_manager.h>
#include <input/sony_controller_input_manager.h>

//combination of conttroler stuff into a class for encapsulation purposes

class Controls
{
protected:

	gef::SonyControllerInputManager* controller_;              //the sony controller input manager, not sure whether needed but is stored just in case
	const gef::SonyController* controls_;                      //the controller container, holds the buttons and axes

	virtual void update() {};                                  //no needed, just available if need be

public:
	Controls(gef::InputManager* input)                         //constructor uses input manager to initialise itself, input manager is not stored
	{
		controller_ = input->controller_input();
		controls_ = controller_->GetController(0);             //only one controller is available, no multiplayer hence index of 0
	}

	~Controls() {};
	void Update();                                                //update function for controller calls the virtual protected update if checks for relevant pointers are correct
	const gef::SonyController* controls() { return controls_; }   //returns the controller container, not needed, but available


	//Functions to simplify the button input queries, masks are set and are used as MACROS essentially

	//Pressed, does not seem to work on gef level
	bool IsTrianglePressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_TRIANGLE; }
	bool IsSquarePressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_SQUARE; }
	bool IsCirclePressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_CIRCLE; }
	bool IsCrossPressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_CROSS; }

	bool IsDPADUPPressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_UP; }
	bool IsDPADDOWNPressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_DOWN; }
	bool IsDPADLEFTPressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_LEFT; }
	bool IsDPADRIGHTPressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_RIGHT; }

	bool IsL2Pressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_L2; }
	bool IsR2Pressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_R2; }
	bool IsL1Pressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_L1; }
	bool IsR1Pressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_R1; }
	bool IsL3Pressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_L3; }
	bool IsR3Pressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_R3; }
	
	bool IsSelectPressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_SELECT; }
	bool IsStartPressed() { return controls_->buttons_pressed() & gef_SONY_CTRL_START; }
	
	//Held, used as the main input query 
	bool IsTriangleHeld() { return controls_->buttons_down() & gef_SONY_CTRL_TRIANGLE; }
	bool IsSquareHeld() { return controls_->buttons_down() & gef_SONY_CTRL_SQUARE; }
	bool IsCircleHeld() { return controls_->buttons_down() & gef_SONY_CTRL_CIRCLE; }
	bool IsCrossHeld() { return controls_->buttons_down() & gef_SONY_CTRL_CROSS; }

	bool IsDPADUPHeld() { return controls_->buttons_down() & gef_SONY_CTRL_UP; }
	bool IsDPADDOWNHeld() { return controls_->buttons_down() & gef_SONY_CTRL_DOWN; }
	bool IsDPADLEFTHeld() { return controls_->buttons_down() & gef_SONY_CTRL_LEFT; }
	bool IsDPADRIGHTHeld() { return controls_->buttons_down() & gef_SONY_CTRL_RIGHT; }

	bool IsL2Held() { return controls_->buttons_down() & gef_SONY_CTRL_L2; }
	bool IsR2Held() { return controls_->buttons_down() & gef_SONY_CTRL_R2; }
	bool IsL1Held() { return controls_->buttons_down() & gef_SONY_CTRL_L1; }
	bool IsR1Held() { return controls_->buttons_down() & gef_SONY_CTRL_R1; }
	bool IsL3Held() { return controls_->buttons_down() & gef_SONY_CTRL_L3; }
	bool IsR3Held() { return controls_->buttons_down() & gef_SONY_CTRL_R3; }

	bool IsSelectHeld() { return controls_->buttons_down() & gef_SONY_CTRL_SELECT; }
	bool IsStartHeld() { return controls_->buttons_down() & gef_SONY_CTRL_START; }

	//Released, does not seem to work on gef level
	bool IsTriangleReleased() { return controls_->buttons_released() & gef_SONY_CTRL_TRIANGLE; }
	bool IsSquareReleased() { return controls_->buttons_released() & gef_SONY_CTRL_SQUARE; }
	bool IsCircleReleased() { return controls_->buttons_released() & gef_SONY_CTRL_CIRCLE; }
	bool IsCrossReleased() { return controls_->buttons_released() & gef_SONY_CTRL_CROSS; }

	bool IsDPADUPReleased() { return controls_->buttons_released() & gef_SONY_CTRL_UP; }
	bool IsDPADDOWNReleased() { return controls_->buttons_released() & gef_SONY_CTRL_DOWN; }
	bool IsDPADLEFTReleased() { return controls_->buttons_released() & gef_SONY_CTRL_LEFT; }
	bool IsDPADRIGHTReleased() { return controls_->buttons_released() & gef_SONY_CTRL_RIGHT; }

	bool IsL2Released() { return controls_->buttons_released() & gef_SONY_CTRL_L2; }
	bool IsR2Released() { return controls_->buttons_released() & gef_SONY_CTRL_R2; }
	bool IsL1Released() { return controls_->buttons_released() & gef_SONY_CTRL_L1; }
	bool IsR1Released() { return controls_->buttons_released() & gef_SONY_CTRL_R1; }
	bool IsL3Released() { return controls_->buttons_released() & gef_SONY_CTRL_L3; }
	bool IsR3Released() { return controls_->buttons_released() & gef_SONY_CTRL_R3; }

	bool IsSelectReleased() { return controls_->buttons_released() & gef_SONY_CTRL_SELECT; }
	bool IsStartReleased() { return controls_->buttons_released() & gef_SONY_CTRL_START; }
};

