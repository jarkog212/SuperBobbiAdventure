#pragma once
#include "audio_emitter.h"

AudioEmitter::AudioEmitter() :
	position_(gef::Vector4::kZero),
	sfx_id_(-1),
	playing_(false),
	voice_id_(-1),
	radius_(1.0f),
	looping_(false)
{
}

void AudioEmitter::Init(int sfx_id, bool looping)
{
	sfx_id_ = sfx_id;
	looping_ = looping;
}

void AudioEmitter::SoundStarted(int voice_num)
{
	voice_id_ = voice_num;
	playing_ = true;
}

void AudioEmitter::SoundStopped()
{
	playing_ = false;
	voice_id_ = -1;
}
