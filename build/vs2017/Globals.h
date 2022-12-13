#pragma once
#include <map>
#include <string>
#include "NetworkGlobals.h"

struct Globals {
	std::map<std::string, int> SFX_map;        //a dictionary translating the SFX names into their IDs
	int G_invert_X_axis = 1;                   //multiplier (either 1 ot -1) for analog input
	int G_invert_Y_axis = 1;                   //...
	int G_master_volume = 100;                 //master volume of the program, changeable only through options state
	int G_music_volume = 20;                   //music volume...
	int G_sfx_volume = 100;                    //SFX volume...
	float G_button_down_timer = 0;             //used to fix an issue with improper controller button state recognition, combined with update, if this value is above 0 no button is registered as held, decrements in update
	bool G_debug_disable_controller = false;
};

