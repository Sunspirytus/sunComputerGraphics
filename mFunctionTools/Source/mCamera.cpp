#include "..\Include\mCamera.h"

Camera::Camera()
{
	
}

Camera::~Camera()
{
}

Camera::Camera(PVRTVec3 Position, PVRTVec3 EulerAngle, PVRTfloat32 fovy, PVRTfloat32 aspect, PVRTfloat32 nearPlane, PVRTfloat32 farPlane, PVRTMat4::eClipspace cs, bool bRotate)
{
	this->Position = Position;
	this->EulerAngle = EulerAngle;
	this->fovy = fovy;
	this->aspect = aspect;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	this->CS = cs;
	this->bRotate = bRotate;
	this->CreateViewMatrixRH();
	this->CreateProjectionMatrixRH();
	this->CreateVPMatrixRH();
}

void Camera::CreateViewMatrixRH()
{
	PVRTMat4 viewMatrix = PVRTMat4::Identity();
	this->rotationMatrix = PVRTMat4::RotationX(EulerAngle.x / 180.0f * PVRT_PI) * PVRTMat4::RotationY(EulerAngle.y / 180.0f * PVRT_PI) * PVRTMat4::RotationZ(EulerAngle.z / 180.0f * PVRT_PI);
	this->translationMatrix = PVRTMat4::Translation(-this->Position);
	this->lookAtMatrix = PVRTMat4::LookAtRH(PVRTVec3(0.0, 0.0, 0.0), this->forward, this->up);
	this->viewMatrix = this->rotationMatrix * this->lookAtMatrix * this->translationMatrix;
	this->forward = this->rotationMatrix * this->forward;
	this->up = this->rotationMatrix * this->up;
	this->left = this->rotationMatrix * this->left;
}

void Camera::CreateProjectionMatrixRH()
{
	this->projectionMatrix = PVRTMat4::PerspectiveFovRH(this->fovy,
		this->aspect,
		this->nearPlane,
		this->farPlane,
		this->CS,
		bRotate);
}

void Camera::CreateVPMatrixRH()
{
	this->VPMatrix = this->projectionMatrix * this->viewMatrix;
}

void Camera::reCreateProjectionMatrix()
{
	this->CreateProjectionMatrixRH();
}

void Camera::ModifyProjectionForClipping(PVRTVec4 vClipPlane)
{
	PVRTVec4 vClipPlaneView(vClipPlane * this->viewMatrix.inverseEx());	// put clip plane into view coords
	/*
	Calculate the clip-space corner point opposite the clipping plane
	and transform it into camera space by multiplying it by the inverse
	projection matrix.
	*/
	PVRTVec4 vClipSpaceCorner(sgn(vClipPlaneView.x), sgn(vClipPlaneView.y), 1.0f, 1.0f);
	vClipSpaceCorner *= this->projectionMatrix.inverseEx();

	// Calculate the scaled plane vector
	PVRTVec4 vScaledPlane = vClipPlaneView * (2.0f / vClipSpaceCorner.dot(vClipPlaneView));

	// Replace the third row of the matrix
	this->projectionMatrix.ptr()[2] = vScaledPlane.x;
	this->projectionMatrix.ptr()[6] = vScaledPlane.y;
	this->projectionMatrix.ptr()[10] = vScaledPlane.z + 1.0f;
	this->projectionMatrix.ptr()[14] = vScaledPlane.w;
	this->CreateVPMatrixRH();
}

void Camera::setPosition(PVRTVec3 position)
{
	if (this->Position == position) return;
	this->translationMatrix = PVRTMat4::Translation(-position);
	this->viewMatrix = this->rotationMatrix	* this->lookAtMatrix * this->translationMatrix;
	this->CreateVPMatrixRH();
	this->Position = position;
}

void Camera::setPosition(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z)
{
	PVRTVec3 newPosition(x, y, z);
	if (this->Position == newPosition) return;
	this->translationMatrix = PVRTMat4::Translation(-newPosition);
	this->viewMatrix = this->rotationMatrix	* this->lookAtMatrix * this->translationMatrix;
	this->CreateVPMatrixRH();
	this->Position = newPosition;
}

void Camera::setEulerAngle(PVRTVec3 eulerAngle)
{
	if (this->EulerAngle == eulerAngle) return;
	this->rotationMatrix = PVRTMat4::RotationZ(eulerAngle.z / 180.0f * PVRT_PI) * PVRTMat4::RotationX(eulerAngle.x / 180.0f * PVRT_PI) * PVRTMat4::RotationY(eulerAngle.y / 180.0f * PVRT_PI);
	this->rotationMatrixForWorld = PVRTMat4::RotationZ(eulerAngle.z / 180.0f * PVRT_PI) * PVRTMat4::RotationX(-eulerAngle.x / 180.0f * PVRT_PI) * PVRTMat4::RotationY(eulerAngle.y / 180.0f * PVRT_PI);
	this->viewMatrix = this->rotationMatrix	* this->lookAtMatrix * this->translationMatrix;
	this->CreateVPMatrixRH();
	this->forward = this->rotationMatrixForWorld * PVRTVec4(0.0, 0.0, 1.0, 0.0);
	this->up = this->rotationMatrixForWorld * PVRTVec4(0.0, 1.0, 0.0, 0.0);
	this->left = this->rotationMatrixForWorld * PVRTVec4(1.0, 0.0, 0.0, 0.0);
	this->EulerAngle = eulerAngle;
}

void Camera::setEulerAngle(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z)
{
	PVRTVec3 newEulerAngle(x, y, z);
	if (this->EulerAngle == newEulerAngle) return;
	this->rotationMatrix = PVRTMat4::RotationZ(z / 180.0f * PVRT_PI)	* PVRTMat4::RotationX(x / 180.0f * PVRT_PI) * PVRTMat4::RotationY(y / 180.0f * PVRT_PI);
	this->rotationMatrixForWorld = PVRTMat4::RotationZ(z / 180.0f * PVRT_PI) * PVRTMat4::RotationX(-x / 180.0f * PVRT_PI) * PVRTMat4::RotationY(y / 180.0f * PVRT_PI);
	this->viewMatrix = this->rotationMatrix	* this->lookAtMatrix * this->translationMatrix;
	this->CreateVPMatrixRH();
	this->forward = this->rotationMatrixForWorld * PVRTVec4(0.0, 0.0, 1.0, 0.0);
	this->up = this->rotationMatrixForWorld * PVRTVec4(0.0, 1.0, 0.0, 0.0);
	this->left = this->rotationMatrixForWorld * PVRTVec4(1.0, 0.0, 0.0, 0.0);
	this->EulerAngle = newEulerAngle;
}

void Camera::setFovy(PVRTfloat32 fovy)
{
	this->fovy = fovy;
	this->CreateProjectionMatrixRH();
	this->CreateVPMatrixRH();
}

void Camera::setAspect(PVRTfloat32 aspect)
{
	this->aspect = aspect;
	this->CreateProjectionMatrixRH();
	this->CreateVPMatrixRH();
}

void Camera::setNearPlaneDis(PVRTfloat32 nearDis)
{
	this->nearPlane = nearDis;
	this->CreateProjectionMatrixRH();
	this->CreateVPMatrixRH();
}

void Camera::setFarPlaneDis(PVRTfloat32 farDis)
{
	this->farPlane = farDis;
	this->CreateProjectionMatrixRH();
	this->CreateVPMatrixRH();
}

void Camera::setClipspace(PVRTMat4::eClipspace cs)
{
	this->CS = cs;
	this->CreateProjectionMatrixRH();
	this->CreateVPMatrixRH();
}

void Camera::setbRotate(bool bRotate)
{
	this->bRotate = bRotate;
	this->CreateProjectionMatrixRH();
	this->CreateVPMatrixRH();
}

PVRTMat4 Camera::getViewMatrix()
{
	return this->viewMatrix;
}

PVRTMat4 Camera::getProjectionMatrix()
{
	return this->projectionMatrix;
}

PVRTMat4 Camera::getVPMatrix()
{
	return this->VPMatrix;
}

PVRTVec3 Camera::getEulerAngle()
{
	return this->EulerAngle;
}

PVRTVec3 Camera::getPosition()
{
	return this->Position;
}

PVRTMat4 Camera::getLookAtMatrix()
{
	return this->lookAtMatrix;
}

PVRTVec3 Camera::getForward()
{
	return this->forward;
}

PVRTVec3 Camera::getUpward()
{
	return this->up;
}

PVRTVec3 Camera::getLeftward()
{
	return this->left;
}

GLfloat Camera::sgn(GLfloat a)
{
	if (a > 0.0f) return(1.0f);
	if (a < 0.0f) return(-1.0f);
	return 0.0f;
}