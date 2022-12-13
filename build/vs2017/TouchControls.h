#pragma once
#include <input/input_manager.h>
#include <input/touch_input_manager.h>

//class to ecapsulate the touch controls

class TouchControls
{
protected:
	gef::TouchInputManager* controller_;

	Int32 active_touch_id_ = -1;             //single touch interface
	const gef::Touch* active_touch_ = NULL;  //touch object pointer
	gef::Vector2 touch_position_;            //position within the window

	virtual void update() {};                //virtual update, not used, can be used to enhance gameplay
	void ProcessTouchInput();                //process touch input

public:
	TouchControls(gef::InputManager* input); 
	

	~TouchControls() {};
	void Update();                                                 //caller for the inward update, has checks

	//getters and setters
	gef::Vector2 touchPosition() { return touch_position_; }       
	Int32 activeTouchID() { return active_touch_id_; }
	const gef::Touch* getActiveTouch() { return active_touch_; }

};
