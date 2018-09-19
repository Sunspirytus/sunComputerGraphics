#include "..\Include\mSuroundBox.h"

mSuroundBox::mSuroundBox()
{

}

mSuroundBox::~mSuroundBox()
{
}

void mSuroundBox::UpdateBoxModel(PVRTuint32 vertexNum, PVRTuint8 * pointPtr, PVRTuint8 stride)
{
	GLfloat x, y, z;
	for (unsigned int i = 0; i < vertexNum; ++i)
	{
		x = *(GLfloat*)(pointPtr + i * stride);
		y = *(GLfloat*)(pointPtr + i * stride + 4);
		z = *(GLfloat*)(pointPtr + i * stride + 8);

		if (x < this->pointMin.x) this->pointMin.x = x;
		if (x > this->pointMax.x) this->pointMax.x = x;
		if (y < this->pointMin.y) this->pointMin.y = y;
		if (y > this->pointMax.y) this->pointMax.y = y;
		if (z < this->pointMin.z) this->pointMin.z = z;
		if (z > this->pointMax.z) this->pointMax.z = z;
	}
	this->CenterModel = (this->pointMin + this->pointMax) / 2;
	this->createCornerPointsModel();
	this->clipPlaneModel[0] = PVRTVec4(1.0, 0.0, 0.0, -this->pointMin.x); //left
	this->clipPlaneModel[1] = PVRTVec4(-1.0, 0.0, 0.0, this->pointMax.x); //right
	this->clipPlaneModel[2] = PVRTVec4(0.0, 1.0, 0.0, -this->pointMin.y); //bottom
	this->clipPlaneModel[3] = PVRTVec4(0.0, -1.0, 0.0, this->pointMax.y); //top
	this->clipPlaneModel[4] = PVRTVec4(0.0, 0.0, 1.0, -this->pointMin.z); //near
	this->clipPlaneModel[5] = PVRTVec4(0.0, 0.0, -1.0, this->pointMax.z); //far
}

void mSuroundBox::UpdateBoxWorld(PVRTMat4 & ModelMatrix)
{
	for (int i = 0; i < 8; ++i)
	{
		this->pointCornersWorld[i] = ModelMatrix * this->pointCornersModel[i];
	}
	for (int i = 0; i < 6; ++i)
	{
		this->clipPlaneWorld[i] = ModelMatrix * this->clipPlaneModel[i];
	}
	this->CenterWorld = ModelMatrix * this->CenterModel;
}

void mSuroundBox::CreateBoxFromCornerWorld(float Xmin, float Xmax, float Ymin, float Ymax, float Zmin, float Zmax)
{
	this->pointMin.x = Xmin;
	this->pointMin.y = Ymin;
	this->pointMin.z = Zmin;
	this->pointMax.x = Xmax;
	this->pointMax.y = Ymax;
	this->pointMax.z = Zmax;
	this->CenterWorld = (this->pointMin + this->pointMax) / 2;
	this->createCornerPointsWorld();
	this->clipPlaneWorld[0] = PVRTVec4(1.0, 0.0, 0.0, -Xmin); //left
	this->clipPlaneWorld[1] = PVRTVec4(-1.0, 0.0, 0.0, Xmax); //right
	this->clipPlaneWorld[2] = PVRTVec4(0.0, 1.0, 0.0, -Ymin); //bottom
	this->clipPlaneWorld[3] = PVRTVec4(0.0, -1.0, 0.0, Ymax); //top
	this->clipPlaneWorld[4] = PVRTVec4(0.0, 0.0, 1.0, -Zmin); //near
	this->clipPlaneWorld[5] = PVRTVec4(0.0, 0.0, -1.0, Zmax); //far
}
/*!****************************************************************************
@Function		HitBox
@Output		HitState		NoHit,	Cross,	Inside,	OutSuround
@Return		int			HitState
@Description	check Hit State, Inside means leftBox is inside Box, 
OutSuround means Box is inside leftBox
******************************************************************************/
int mSuroundBox::HitBox(mSuroundBox & Box)
{
	int sSize = 0;
	int bSize = 0;
	for (int i = 0; i < 6; ++i){
		bSize = 0;
		if (Box.clipPlaneWorld[i].dot(this->pointCornersWorld[0]) <= 0) sSize++;	else bSize++;
		if (Box.clipPlaneWorld[i].dot(this->pointCornersWorld[1]) <= 0) sSize++;	else bSize++;
		if (Box.clipPlaneWorld[i].dot(this->pointCornersWorld[2]) <= 0) sSize++;	else bSize++;
		if (Box.clipPlaneWorld[i].dot(this->pointCornersWorld[3]) <= 0) sSize++;	else bSize++;
		if (Box.clipPlaneWorld[i].dot(this->pointCornersWorld[4]) <= 0) sSize++;	else bSize++;
		if (Box.clipPlaneWorld[i].dot(this->pointCornersWorld[5]) <= 0) sSize++;	else bSize++;
		if (Box.clipPlaneWorld[i].dot(this->pointCornersWorld[6]) <= 0) sSize++;	else bSize++;
		if (Box.clipPlaneWorld[i].dot(this->pointCornersWorld[7]) <= 0) sSize++;	else bSize++;
		if (bSize == 0){
			return NoHit;
		}
	}
	if (sSize == 0){
		return Inside;
	}

	sSize = 0;
	bSize = 0;
	for (int i = 0; i < 6; ++i){
		bSize = 0;
		if (this->clipPlaneWorld[i].dot(Box.pointCornersWorld[0]) <= 0) sSize++;	else bSize++;
		if (this->clipPlaneWorld[i].dot(Box.pointCornersWorld[1]) <= 0) sSize++;	else bSize++;
		if (this->clipPlaneWorld[i].dot(Box.pointCornersWorld[2]) <= 0) sSize++;	else bSize++;
		if (this->clipPlaneWorld[i].dot(Box.pointCornersWorld[3]) <= 0) sSize++;	else bSize++;
		if (this->clipPlaneWorld[i].dot(Box.pointCornersWorld[4]) <= 0) sSize++;	else bSize++;
		if (this->clipPlaneWorld[i].dot(Box.pointCornersWorld[5]) <= 0) sSize++;	else bSize++;
		if (this->clipPlaneWorld[i].dot(Box.pointCornersWorld[6]) <= 0) sSize++;	else bSize++;
		if (this->clipPlaneWorld[i].dot(Box.pointCornersWorld[7]) <= 0) sSize++;	else bSize++;
		if (bSize == 0){
			return NoHit;
		}
	}
	if (sSize == 0){
		return OutSuround;
	}

	return Cross;
}

bool mSuroundBox::NeedClipFromObjSpace(PVRTMat4 & MVP_Matrix)
{
	PVRTVec4 clipPlanes[6];
	PVRTMat4 MVP_Matrix_T = MVP_Matrix.transpose();
	PVRTVec4 row1 = MVP_Matrix_T[0];
	PVRTVec4 row2 = MVP_Matrix_T[1];
	PVRTVec4 row3 = MVP_Matrix_T[2];
	PVRTVec4 row4 = MVP_Matrix_T[3];
	clipPlanes[0] = row1 + row4; //left
	clipPlanes[1] = row4 - row1; //right
	clipPlanes[2] = row2 + row4; //bottom
	clipPlanes[3] = row4 - row2; //top
	clipPlanes[4] = row3 + row4; //near
	clipPlanes[5] = row4 - row3; //far
	
	for (int i = 0; i < 6; ++i){
		if (clipPlanes[i].dot(this->pointCornersModel[0]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersModel[1]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersModel[2]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersModel[3]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersModel[4]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersModel[5]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersModel[6]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersModel[7]) > 0) continue;
		return true;
	}
	return false;
}

bool mSuroundBox::NeedClipFromWorldSpace(PVRTMat4 & VP_Matrix)
{
	PVRTVec4 clipPlanes[6];
	PVRTMat4 VP_Matrix_T = VP_Matrix.transpose();
	PVRTVec4 row1 = VP_Matrix_T[0];
	PVRTVec4 row2 = VP_Matrix_T[1];
	PVRTVec4 row3 = VP_Matrix_T[2];
	PVRTVec4 row4 = VP_Matrix_T[3];
	clipPlanes[0] = row1 + row4; //left
	clipPlanes[1] = row4 - row1; //right
	clipPlanes[2] = row2 + row4; //bottom
	clipPlanes[3] = row4 - row2; //top
	clipPlanes[4] = row3 + row4; //near
	clipPlanes[5] = row4 - row3; //far

	for (int i = 0; i < 6; ++i){
		if (clipPlanes[i].dot(this->pointCornersWorld[0]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersWorld[1]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersWorld[2]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersWorld[3]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersWorld[4]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersWorld[5]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersWorld[6]) > 0) continue;
		if (clipPlanes[i].dot(this->pointCornersWorld[7]) > 0) continue;
		return true;
	}
	return false;
}

bool mSuroundBox::CenterInsideBoxWorldSpace(mSuroundBox & Box)
{
	if (this->CenterWorld.dot(Box.clipPlaneWorld[0]) <= 0) return false;
	if (this->CenterWorld.dot(Box.clipPlaneWorld[1]) < 0) return false;
	if (this->CenterWorld.dot(Box.clipPlaneWorld[2]) <= 0) return false;
	if (this->CenterWorld.dot(Box.clipPlaneWorld[3]) < 0) return false;
	if (this->CenterWorld.dot(Box.clipPlaneWorld[4]) <= 0) return false;
	if (this->CenterWorld.dot(Box.clipPlaneWorld[5]) < 0) return false;
	return true;
}

void mSuroundBox::createCornerPointsModel()
{
	this->pointCornersModel[0] = PVRTVec4(this->pointMin.x, this->pointMin.y, this->pointMin.z, 1.0);
	this->pointCornersModel[1] = PVRTVec4(this->pointMax.x, this->pointMin.y, this->pointMin.z, 1.0);
	this->pointCornersModel[2] = PVRTVec4(this->pointMin.x, this->pointMax.y, this->pointMin.z, 1.0);
	this->pointCornersModel[3] = PVRTVec4(this->pointMax.x, this->pointMax.y, this->pointMin.z, 1.0);
	this->pointCornersModel[4] = PVRTVec4(this->pointMin.x, this->pointMin.y, this->pointMax.z, 1.0);
	this->pointCornersModel[5] = PVRTVec4(this->pointMax.x, this->pointMin.y, this->pointMax.z, 1.0);
	this->pointCornersModel[6] = PVRTVec4(this->pointMin.x, this->pointMax.y, this->pointMax.z, 1.0);
	this->pointCornersModel[7] = PVRTVec4(this->pointMax.x, this->pointMax.y, this->pointMax.z, 1.0);
}

void mSuroundBox::createCornerPointsWorld()
{
	this->pointCornersWorld[0] = PVRTVec4(this->pointMin.x, this->pointMin.y, this->pointMin.z, 1.0);
	this->pointCornersWorld[1] = PVRTVec4(this->pointMax.x, this->pointMin.y, this->pointMin.z, 1.0);
	this->pointCornersWorld[2] = PVRTVec4(this->pointMin.x, this->pointMax.y, this->pointMin.z, 1.0);
	this->pointCornersWorld[3] = PVRTVec4(this->pointMax.x, this->pointMax.y, this->pointMin.z, 1.0);
	this->pointCornersWorld[4] = PVRTVec4(this->pointMin.x, this->pointMin.y, this->pointMax.z, 1.0);
	this->pointCornersWorld[5] = PVRTVec4(this->pointMax.x, this->pointMin.y, this->pointMax.z, 1.0);
	this->pointCornersWorld[6] = PVRTVec4(this->pointMin.x, this->pointMax.y, this->pointMax.z, 1.0);
	this->pointCornersWorld[7] = PVRTVec4(this->pointMax.x, this->pointMax.y, this->pointMax.z, 1.0);
}