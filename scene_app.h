#ifndef _SCENE_APP_H
#define _SCENE_APP_H

#pragma once
#include <system/application.h>
#include <maths/vector2.h>
#include "primitive_builder.h"
#include <graphics/mesh_instance.h>
#include <audio/audio_manager.h>
#include <input/input_manager.h>
#include <box2d/Box2D.h>
#include "audio_3d.h"
#include <map>
#include <string>

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
	class Renderer3D;
}
class SceneManager;

class SceneApp : public gef::Application
{
public:
	SceneApp(gef::Platform& platform);
	void Init();
	void CleanUp();
	bool Update(float frame_time);
	void Render();
private:
	void InitFont();
	void CleanUpFont();
	void InitStateMachine();
	void InitSound();

    
	gef::SpriteRenderer* sprite_renderer_;
	gef::Font* font_;
	gef::Renderer3D* renderer_3d_;

	PrimitiveBuilder* primitive_builder_;
	gef::InputManager* input_manager_;
	gef::AudioManager* audio_manager_;

	int sfx_id_;

	Audio3D* audio_3d_;
	SceneManager* stateMachine_;

	float fps_;
};

#endif // _SCENE_APP_H
