#pragma once
#include <input/input_manager.h>

//class to encapsulate the keyboard controls

class KeyboardControls
{
	const gef::Keyboard* keyboard_;   //pointer to the keyboard container class

	virtual void update() {};         //ptential use to add functionality like input changed by gameplay

public:
	KeyboardControls(gef::InputManager* input)
	{
		keyboard_ = input->keyboard();
	}
	~KeyboardControls() {}; 
	void Update();                                        //caller for virtual update, has checks
	
    //getter
	const gef::Keyboard* controls() { return keyboard_; } 
};

