#ifndef _AUDIO_3D_H
#define _AUDIO_3D_H

#pragma once
#include <audio/audio_manager.h>
#include "audio_listener.h"
#include "audio_emitter.h"
#include <vector>

class Audio3D
{
public:
	Audio3D(gef::AudioManager* audio_manager);
	~Audio3D();

	void Update();

	void AddEmitter(AudioEmitter* emitter);
	void SetListener(AudioListener* listener) { listener_ = listener; };
	inline AudioListener* listener() { return listener_; }
	void deleteListener();
	gef::AudioManager* getAudioManager() { return audio_manager_; }
private:
	gef::AudioManager* audio_manager_;
	AudioListener* listener_;

	std::vector<AudioEmitter*> emitters_;
//	AudioEmitter emitters_[20];
};

#endif // _AUDIO_3D_H