#pragma once
#include "KeyboardControls.h"

//update for keyboard with checks, delegate
void KeyboardControls::Update()
{
	if (keyboard_) {
		update();
	}
}
