#pragma once
#include "audio_listener.h"

AudioListener::AudioListener()
{
	// initialise transform to the identity matrix
	transform_.SetIdentity();

	// the inverse of the identity matrix is the identity matrix
	// so just do a straight copy
	inv_transform_ = transform_;
}

void AudioListener::SetTransform(const gef::Matrix44& transform)
{
	transform_ = transform;
	inv_transform_.Inverse(transform_);
}

gef::Vector4 AudioListener::TransformWorld2Local(const gef::Vector4& world_position)
{
	gef::Vector4 local_position = world_position.Transform(inv_transform_);
	return local_position;
}
