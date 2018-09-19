#include "..\Include\mModel.h"


mModel::mModel()
{
	this->RotationMatrix = PVRTMat4::Identity();
	this->ModelMatrix = PVRTMat4::Identity();
}

mModel::~mModel()
{	
	
}

void mModel::Destroy()
{
	this->ModelPOD->Destroy();
	delete[] this->VBO;
	delete[] this->IndexVBO;
	this->ModelPOD = nullptr;
	this->VBO = nullptr;
	this->IndexVBO = nullptr;
}

void mModel::DeleteVBOs()
{
	glDeleteBuffers(this->ModelPOD->nNumMesh, this->VBO);
	glDeleteBuffers(this->ModelPOD->nNumMesh, this->IndexVBO);
}

void mModel::SetPOD(CPVRTModelPOD * modelPOD)
{
	this->ModelPOD = modelPOD;
}

void mModel::SetTransform(PVRTVec3 Position, PVRTVec3 EulerAngle, PVRTVec3 Scal)
{
	this->RotationMatrix = PVRTMat4::RotationX(EulerAngle.x) * PVRTMat4::RotationY(EulerAngle.y) * PVRTMat4::RotationZ(EulerAngle.z);
	this->ModelMatrix = this->RotationMatrix;
	this->ModelMatrix.ptr()[12] = Position.x;
	this->ModelMatrix.ptr()[13] = Position.y;
	this->ModelMatrix.ptr()[14] = Position.z;
	this->ModelMatrix.ptr()[0] *= Scale.x;
	this->ModelMatrix.ptr()[5] *= Scale.y;
	this->ModelMatrix.ptr()[10] *= Scale.z;
	this->Position = Position;
	this->EulerAngle = EulerAngle;
	this->Scale = Scale;
}

void mModel::LoadVBO()
{
	this->VBO = new GLuint[this->ModelPOD->nNumMesh];
	this->IndexVBO = new GLuint[this->ModelPOD->nNumMesh];
	glGenBuffers(this->ModelPOD->nNumMesh, this->VBO);
	for (unsigned int i = 0; i < this->ModelPOD->nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = this->ModelPOD->pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		this->IndexVBO[i] = 0;
		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &this->IndexVBO[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IndexVBO[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}

	}
}

void mModel::SetPosition(PVRTVec3 position)
{
	if (this->Position == position) return;
	this->Position = position;
	this->UpdatePosition();
	this->SurrondBox.UpdateBoxWorld(this->ModelMatrix);
}

void mModel::SetPosition(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z)
{
	PVRTVec3 newPosition = PVRTVec3(x, y, z);
	if (this->Position == newPosition) return;
	this->Position = newPosition;
	this->UpdatePosition();
	this->SurrondBox.UpdateBoxWorld(this->ModelMatrix);
}

void mModel::SetEulerAngle(PVRTVec3 eulerAngle)
{
	if (this->EulerAngle == eulerAngle) return;
	this->EulerAngle = eulerAngle;
	this->UpdateRotateion();
	this->ModelMatrix = this->RotationMatrix;
	this->UpdatePosition();
	this->UpdateScale();
	this->SurrondBox.UpdateBoxWorld(this->ModelMatrix);
}

void mModel::SetEulerAngle(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z)
{
	PVRTVec3 newEulerAngle = PVRTVec3(x, y, z);
	if (this->EulerAngle == newEulerAngle) return;
	this->EulerAngle = newEulerAngle;
	this->UpdateRotateion();
	this->ModelMatrix = this->RotationMatrix;
	this->UpdatePosition();
	this->UpdateScale();
	this->SurrondBox.UpdateBoxWorld(this->ModelMatrix);
}

void mModel::SetScale(PVRTVec3 scale)
{
	if (this->Scale == scale) return;
	this->ChangeScale(scale);
	this->Scale = scale;
	this->SurrondBox.UpdateBoxWorld(this->ModelMatrix);
}

void mModel::SetScale(PVRTfloat32 x, PVRTfloat32 y, PVRTfloat32 z)
{
	PVRTVec3 newScale = PVRTVec3(x, y, z);
	if (this->Scale == newScale) return;
	this->ChangeScale(newScale);
	this->Scale = newScale;
	this->SurrondBox.UpdateBoxWorld(this->ModelMatrix);
}

void mModel::CreateSuroundBox()
{
	for (unsigned int i = 0; i < this->ModelPOD->nNumMesh; ++i)
	{
		SPODMesh& Mesh = this->ModelPOD->pMesh[i];
		this->SurrondBox.UpdateBoxModel(Mesh.nNumVertex, Mesh.pInterleaved, Mesh.sVertex.nStride);
	}
}

bool mModel::NeedClip(PVRTMat4 & MVPmatrix)
{
	return this->SurrondBox.NeedClipFromObjSpace(MVPmatrix);
}

PVRTMat4 mModel::GetModelMatrix()
{
	return this->ModelMatrix;
}

void mModel::UpdatePosition()
{
	this->ModelMatrix.ptr()[12] = this->Position.x;
	this->ModelMatrix.ptr()[13] = this->Position.y;
	this->ModelMatrix.ptr()[14] = this->Position.z;
}

void mModel::UpdateRotateion()
{
	this->RotationMatrix = PVRTMat4::RotationX(this->EulerAngle.x / 180.0f / PVRT_PI) * PVRTMat4::RotationY(this->EulerAngle.y / 180.0f / PVRT_PI) * PVRTMat4::RotationZ(this->EulerAngle.z / 180.0f / PVRT_PI);

}

void mModel::ChangeScale(PVRTVec3 scale)
{
	this->ModelMatrix.ptr()[0] *= scale.x / this->Scale.x;
	this->ModelMatrix.ptr()[5] *= scale.y / this->Scale.y;
	this->ModelMatrix.ptr()[10] *= scale.z / this->Scale.z;
}

void mModel::UpdateScale()
{
	this->ModelMatrix.ptr()[0] *= this->Scale.x;
	this->ModelMatrix.ptr()[5] *= this->Scale.y;
	this->ModelMatrix.ptr()[10] *= this->Scale.z;
}

PVRTVec3 mModel::GetPosition()
{
	return this->Position;
}