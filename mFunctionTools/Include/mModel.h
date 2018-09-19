#ifndef __MMODEL_H_
#define __MMODEL_H_

#include "PVRShell.h"
#include "OGLES2Tools.h"
#include "mSuroundBox.h"

class mModel
{
public:
	mModel();
	~mModel();

	mSuroundBox SurrondBox;
	CPVRTModelPOD * ModelPOD = nullptr;
	GLuint * VBO = nullptr;
	GLuint * IndexVBO = nullptr;

	bool needRender = false;

	void LoadVBO();
	void CreateSuroundBox();
	void SetPOD(CPVRTModelPOD * modelPOD);
	void SetTransform(PVRTVec3 Position, PVRTVec3 EulerAngle, PVRTVec3 Scale);
	void SetPosition(PVRTVec3 position);
	void SetPosition(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z);
	void SetEulerAngle(PVRTVec3 eulerAngle);
	void SetEulerAngle(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z);
	void SetScale(PVRTVec3 scale);
	void SetScale(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z);
	PVRTVec3 GetPosition();
	PVRTMat4 GetModelMatrix();
	bool NeedClip(PVRTMat4 & MVPmatrix);

	void DeleteVBOs();
	void Destroy();

private:
	PVRTVec3 Position = PVRTVec3(0.0, 0.0, 0.0);
	PVRTVec3 EulerAngle = PVRTVec3(0.0, 0.0, 0.0);
	PVRTVec3 Scale = PVRTVec3(1.0, 1.0, 1.0);
	PVRTMat4 RotationMatrix;
	PVRTMat4 ModelMatrix;

	void UpdatePosition();
	void UpdateRotateion();
	void UpdateScale();
	void ChangeScale(PVRTVec3 scale);
};
#endif