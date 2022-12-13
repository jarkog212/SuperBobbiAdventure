#pragma once
#include "SplashState.h"
#include <audio/audio_manager.h>

//update override
void SplashState::update(float deltaT)
{
	if (elapsed_time_ >= SPLASH_DURATION_S) {                  //wait for the timer, then initiate switch of the state to menu
		state_switch_data_.needs_to_change = true;
		state_switch_data_.new_kept_state_ = GameState::MENU;
	}
	elapsed_time_ += deltaT;
}

//override, play the splash music, looping yes but should not loop due to the timer
void SplashState::PlayStateBGM()
{
	hasBGM_ = true;
	audio_->LoadMusic("BGM/splash.wav", *platform_);
	audio_->PlayMusic();
}
