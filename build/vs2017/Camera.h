#pragma once
#include <maths/matrix44.h>

using gef::Matrix44;
using gef::Vector4;

namespace gef
{
	class InputManager;
	class Renderer3D;
}

class Controls;

//grouping of frustum, screen parameters

struct ProjectionParams {
	float left = 0;                     //frustum params
	float right = 0;                    //...
	float top = 0;                      
	float bottom = 0;
	float near_dist = 0;                
	float far_dist = 0;
	float fov = 0;                      //field of view angle
	float aspect_ratio = 16.0f/9.0f;    //aspect ratio for the screen (widescreen standard is 16:9)
};

// grouping of the view matrix, spearated for easier use, eye is position, lookat is forward

struct LookAtParams {
	Vector4 eye = Vector4(0,0,0,0);
	Vector4 lookat = Vector4(0,0,0,0);
	Vector4 up = Vector4(0, 0, 0, 0);
};

//camera object encapsulating the behaviours and variables of the camera

class Camera
{
	Matrix44 projection_;           //final matrix of the frustum and stuff
	Matrix44 view_;                 //final matrix of the view 
	ProjectionParams proj_p_;       
	LookAtParams look_at_p_;
	Vector4 forward_;               //currently unnused, could be saved for first person camera
	Vector4 right_;                 //...
	Vector4 up_;                    //... deprecated, kindof, as is a duplicate of look at params's up
	Vector4 position_;              //...                                               ...params's eye
	gef::Renderer3D* renderer_;
	Controls* controller_;          // potential use in other type of game, first person

	void init(gef::Renderer3D* renderer, ProjectionParams proj_p, LookAtParams look_at_p);   //is separate from constructor, loading pusposes
	virtual void handleIn() {};                                                              //function for input, if any would be used
	void moveForward(float scale) {};                                                        //functions for controlling the camera, not used in the current game...
	void moveRight(float scale) {};                                                          //...     
	void rotate(float x_rot_pitch, float y_rot_yaw) {};                                      //...
	 
public:
	Camera(gef::Renderer3D* renderer);                                                       //camera has to have a renderer, otherwie it cannot work, hence no basic constructor
	Camera(gef::Renderer3D* renderer,ProjectionParams proj_p, LookAtParams look_at_p);
	~Camera() {};
	void setFrustum(ProjectionParams proj_p);                                                //set, or change the frustum params, could be used for cool visual effects
	void setView(LookAtParams look_at_p);                                                    //sets the "transform" of th ecamera
	void handleInput();                                                                      //wrapping function of handle input, calls the virtual handle in after checks, checks are needed for all potential children hence this model
	void apply();                                                                            //apply the camera, call the renderers setView function with the camera data
	void setInput(gef::InputManager* in);                                                    //function that creates controller and connects it to the input manager

	LookAtParams getView() { return look_at_p_; }                                            //essentially getTransform

};

