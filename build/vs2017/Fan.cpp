#pragma once
#include "Fan.h"
#include "PlayerBobbi.h"
#include <graphics/renderer_3d.h>
#include "Globals.h"

#define DISTANCE_FROM_PLAYER 1.0f                                       //how far from the player fan is shown 

//handle input
void Fan::handleControllerInput(float dT)                            
{
	if (controller_input_->controls()->left_stick_x_axis() == 0 &&      //if left stick is not moved and visible...
		controller_input_->controls()->left_stick_y_axis() == 0 &&
		getVisible())                                                   
	{
		toggleVisibility();                                             //...set as not visible
	}
	else if((controller_input_->controls()->left_stick_x_axis() != 0 || //if left stick is moved and is not visible...
			controller_input_->controls()->left_stick_y_axis() != 0) &&
			!getVisible())
	{
		toggleVisibility();                                             //...set as visible
	}

	setPosition(calculatePositionFromPlayer());                         //calculate the position from the player

	if (controller_input_->IsR2Held()) {                                //if the R2 button is pressed...
		Rotate(Vector4(0, 0, 3680 * dT));                               //... rotate the fan
	}
}

//external link to the gloabal variables
extern Globals globals;

//calculate the position relative to the player
Vector4 Fan::calculatePositionFromPlayer()
{
	
	// calculates the wind vector direction from player and applies the inverse setting 
	Vector4 temp = player_->getTransform().position_;                                                                  
	Vector4 wind_vector = Vector4(controller_input_->controls()->left_stick_x_axis() *globals.G_invert_X_axis* DISTANCE_FROM_PLAYER, controller_input_->controls()->left_stick_y_axis()* -1 *globals.G_invert_Y_axis* DISTANCE_FROM_PLAYER, 0, 0);
    
	temp += wind_vector;                //adds the wind vector offset to the player position, creating the fan world position
	temp.set_z(0);                      //set the z position as 0
	return temp;                         
}

//constructor, requires player pointer
Fan::Fan(gef::Platform& platform, gef::InputManager* input, gef::Renderer3D* renderer, PlayerBobbi* player) : GameObject_3D(platform, "Models/Fan.scn", renderer)
{
	setupControllerInput(input);
	player_ = player;
}

//render
void Fan::RenderObject()
{
	if (is_visible) {                                                                              //if visible...
		gef::Colour temp_ = renderer_->default_shader_data().ambient_light_colour();               //...save light data...
		renderer_->default_shader_data().set_ambient_light_colour(gef::Colour(0.7, 0.7, 0.7));     //...disables light...
		applyTransform();
		renderer_->DrawMesh(*this);
		renderer_->default_shader_data().set_ambient_light_colour(temp_);                          //...reapplies light
	}

}
