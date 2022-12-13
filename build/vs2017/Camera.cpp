#pragma once
#include "Camera.h"
#include <graphics/renderer_3d.h>
#include "Controls.h"

//intialisation
void Camera::init(gef::Renderer3D* renderer, ProjectionParams proj_p, LookAtParams look_at_p)
{
	renderer_ = renderer;
	setFrustum(proj_p);                                             //sets the frustum parameters
	setView(look_at_p);                                             //sets the original view, used for initialisation purposes
	position_ = look_at_p.eye;                                      //save sthe position in a separate variable, used if camera would be controllable
	forward_ = look_at_p.lookat - look_at_p.eye;                    //calculate the forward vector, used if camera would be controllable
	up_ = look_at_p.up;                                             //save up as separate, used if camera would be controllable
	right_ = forward_.CrossProduct(up_);                            //calculates the right vector, would be used if cammera is controllable
}

//constructor
Camera::Camera(gef::Renderer3D* renderer)
{
	ProjectionParams proj_p;                        
	LookAtParams look_at_p;
	init(renderer, proj_p, look_at_p);
}

//constructor with details
Camera::Camera(gef::Renderer3D* renderer, ProjectionParams proj_p, LookAtParams look_at_p)
{
	init(renderer, proj_p, look_at_p);
}

//set frustum details, if needed can be changed during gameplay for visual effects
void Camera::setFrustum(ProjectionParams proj_p)
{
	if (proj_p.fov != 0)                               //if fov is not specified use other available dat to set up frustum
		projection_.PerspectiveFovD3D(proj_p.fov, proj_p.aspect_ratio, proj_p.near_dist, proj_p.far_dist);
	else                                               //if fov is available, use it to calculate the frustum parameters
		projection_.PerspectiveFrustumD3D(proj_p.left, proj_p.right, proj_p.top, proj_p.bottom, proj_p.near_dist, proj_p.far_dist);

	proj_p_ = proj_p;   //store the data
}

//sets the view, then the view is propagated into renderer
void Camera::setView(LookAtParams look_at_p)
{
	view_.LookAt(look_at_p.eye, look_at_p.lookat, look_at_p.up);  
	look_at_p_ = look_at_p;
}

//handle inut of the camera, not used as camera is not controllable
void Camera::handleInput()
{
	if (controller_)
		handleIn();
}

//applies the stored view into renderer
void Camera::apply()
{
	renderer_->set_view_matrix(view_);
	renderer_->set_projection_matrix(projection_);
}

//creates a new controlller uses input manager
void Camera::setInput(gef::InputManager* in)
{
	controller_ = new Controls(in);
}
