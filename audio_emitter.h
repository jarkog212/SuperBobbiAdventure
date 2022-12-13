#ifndef _AUDIO_EMITTER_H
#define _AUDIO_EMITTER_H

#pragma once
#include <maths/vector4.h>

class AudioEmitter
{
public:
	AudioEmitter();
	void Init(int sfx_id, bool looping = false);
	void SoundStarted(int voice_num);
	void SoundStopped();

	inline bool playing() const { return playing_; }

	inline const gef::Vector4& position() const { return position_;	}
	inline void set_position(const gef::Vector4& position) { position_ = position; }

	inline float radius() const { return radius_; }
	inline void set_radius(float radius) { radius_ = radius; }

	inline int sfx_id() const { return sfx_id_; }
	inline int voice_id() const { return voice_id_; }

	inline bool looping() const { return looping_; }
	inline void set_looping(bool looping) { looping_ = looping; }
private:
	gef::Vector4 position_;
	bool playing_;
	int voice_id_;
	int sfx_id_;
	float radius_;
	bool looping_;
};

#endif // _AUDIO_EMITTER_H
