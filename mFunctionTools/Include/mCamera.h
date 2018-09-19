#ifndef __MCAMERA_H_
#define __MCAMERA_H_

#include "PVRShell.h"
#include "OGLES2Tools.h"

class Camera
{
public:
	Camera();
	~Camera();
	Camera(PVRTVec3 Position,
		PVRTVec3 EulerAngle,
		PVRTfloat32 fovy,
		PVRTfloat32 aspect,
		PVRTfloat32 nearPlane,
		PVRTfloat32 farPlane,
		PVRTMat4::eClipspace cs,
		bool bRotate = false);
	void ModifyProjectionForClipping(PVRTVec4 vClipPlane);
	void reCreateProjectionMatrix();

	PVRTMat4 getViewMatrix();
	PVRTMat4 getProjectionMatrix();
	PVRTMat4 getLookAtMatrix();
	PVRTMat4 getVPMatrix();
	PVRTVec3 getEulerAngle();
	PVRTVec3 getPosition();
	PVRTVec3 getForward();
	PVRTVec3 getUpward();
	PVRTVec3 getLeftward();
	void setPosition(PVRTVec3 position);
	void setPosition(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z);
	void setEulerAngle(PVRTVec3 eulerAngle);
	void setEulerAngle(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z);
	void setFovy(PVRTfloat32 fovy);
	void setAspect(PVRTfloat32 aspect);
	void setNearPlaneDis(PVRTfloat32 nearDis);
	void setFarPlaneDis(PVRTfloat32 farDis);
	void setClipspace(PVRTMat4::eClipspace cs);
	void setbRotate(bool bRotate);

private:
	PVRTVec3 Position;
	PVRTVec3 EulerAngle;
	PVRTfloat32 fovy;
	PVRTfloat32 aspect;
	PVRTfloat32 nearPlane;
	PVRTfloat32 farPlane;
	PVRTVec4 forward = PVRTVec4(0.0, 0.0, 1.0, 0.0);
	PVRTVec4 up = PVRTVec4(0.0, 1.0, 0.0, 0.0);
	PVRTVec4 left = PVRTVec4(1.0, 0.0, 0.0, 0.0);
	PVRTMat4::eClipspace CS; //OPENGL OR DX
	bool bRotate;

	void CreateViewMatrixRH();
	void CreateProjectionMatrixRH();
	void CreateVPMatrixRH();
	PVRTMat4 translationMatrix;
	PVRTMat4 rotationMatrix;
	PVRTMat4 rotationMatrixForWorld;
	PVRTMat4 viewMatrix;
	PVRTMat4 lookAtMatrix;
	PVRTMat4 projectionMatrix;
	PVRTMat4 VPMatrix;
	GLfloat sgn(GLfloat a);
};


#endif