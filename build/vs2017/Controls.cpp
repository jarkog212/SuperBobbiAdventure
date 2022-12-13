#pragma once
#include "Controls.h"

//update
void Controls::Update()
{
	if (controller_ && controls_) {             //checks whether pointers are intialised
		update();                               //calls update
	}
}
