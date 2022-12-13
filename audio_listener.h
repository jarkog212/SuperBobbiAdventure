#ifndef _AUDIO_LISTENER_H
#define _AUDIO_LISTENER_H

#pragma once
#include <maths/vector4.h>
#include <maths/matrix44.h>

class AudioListener
{
public:
	AudioListener();

	void SetTransform(const gef::Matrix44& transform);
	gef::Vector4 TransformWorld2Local(const gef::Vector4& world_position);

private:
	gef::Matrix44 transform_;
	gef::Matrix44 inv_transform_;
};

#endif // _AUDIO_LISTENER_H