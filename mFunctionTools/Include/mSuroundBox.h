#ifndef __MSUROUNDBOX_H_
#define __MSUROUNDBOX_H_

#include "PVRShell.h"
#include "OGLES2Tools.h"
#include <float.h>

//            2                                3
//              ------------------------------
//             /|                           /|
//            / |                          / |				ÓÒÊÖ×ø±êÏµ
//           /  |                         /  |
//          /   |                        /   |
//         /    |                       /    |				+Y
//        /     |                      /     |				|
//       /      |                     /      |				|
//      /       |                    /       |				|
//     /        |                   /        |				|
//  6 /         |                7 /         |				|
//   /----------------------------/          |				/-------------- +X
//   |          |                 |          |			   /	
//   |          |                 |          |      	  /
//   |        0 |                 |          |			 /
//   |          |-----------------|----------|			/+Z
//   |         /                  |         /  1    
//   |        /                   |        /               
//   |       /                    |       /         
//   |      /                     |      /              
//   |     /                      |     /               
//   |    /                       |    /               
//   |   /                        |   /           
//   |  /                         |  /            
//   | /                          | /             
//   |/                           |/              
//   ------------------------------               
//  4                              5
//

enum HitState
{
	NoHit,
	Cross,
	Inside,
	OutSuround
};

class mSuroundBox
{
public:
	mSuroundBox();
	~mSuroundBox();

	void UpdateBoxModel(PVRTuint32 vertexNum, PVRTuint8 * pointPtr, PVRTuint8 stride);
	void UpdateBoxWorld(PVRTMat4 & ModelMatrix);
	void CreateBoxFromCornerWorld(float Xmin, float Xmax, float Ymin, float Ymax, float Zmin, float Zmax);
	bool NeedClipFromObjSpace(PVRTMat4 & MVP_Matrix);
	bool NeedClipFromWorldSpace(PVRTMat4 & VP_Matrix);
	int HitBox(mSuroundBox & Box);
	bool CenterInsideBoxWorldSpace(mSuroundBox & Box);

	PVRTVec4 pointMin = PVRTVec4(FLT_MAX, FLT_MAX, FLT_MAX, 1.0);
	PVRTVec4 pointMax = PVRTVec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0);

private:
	PVRTVec4 CenterModel;
	PVRTVec4 CenterWorld;
	PVRTVec4 pointCornersModel[8];
	PVRTVec4 clipPlaneModel[6];

	PVRTVec4 pointCornersWorld[8];
	PVRTVec4 clipPlaneWorld[6];
	void createCornerPointsModel();
	void createCornerPointsWorld();
};




#endif