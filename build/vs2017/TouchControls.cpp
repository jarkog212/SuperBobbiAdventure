#pragma once
#include "TouchControls.h"

//code by lecturers, process touch input
void TouchControls::ProcessTouchInput()
{
	if (controller_ && (controller_->max_num_panels() > 0))
	{
		// get the active touches for this panel (only works with single in)
		const gef::TouchContainer& panel_touches = controller_->touches(0);

		// go through the touches
		for (gef::ConstTouchIterator touch = panel_touches.begin(); touch != panel_touches.end(); ++touch)
		{
			// if active touch id is -1, then we are not currently processing a touch
			if (active_touch_id_ == -1)
			{
				// check for the start of a new touch
				if (touch->type == gef::TT_NEW)
				{
					active_touch_id_ = touch->id;
					active_touch_ = &(*touch);

					// do any processing for a new touch here
					// we're just going to record the position of the touch
					touch_position_ = touch->position;
				}
			}
			else if (active_touch_id_ == touch->id)
			{
				// we are processing touch data with a matching id to the one we are looking for
				if (touch->type == gef::TT_ACTIVE)
				{
					// update an active touch here
					// we're just going to record the position of the touch
					touch_position_ = touch->position;
				}
				else if (touch->type == gef::TT_RELEASED)
				{
					// the touch we are tracking has been released
					// perform any actions that need to happen when a touch is released here
					// we're not doing anything here apart from resetting the active touch id
					active_touch_id_ = -1;
				}
			}
		}
	}
}

//constructor 
TouchControls::TouchControls(gef::InputManager* input)
{
	if (input && input->touch_manager() && (input->touch_manager()->max_num_panels() > 0))
	{
		input->touch_manager()->EnablePanel(0);
		controller_ = input->touch_manager();
	}
}

//calls update if there is a gameplay reason calls the processing of touch
void TouchControls::Update()
{
	if (controller_) {
		update();
		ProcessTouchInput();
	}
}
