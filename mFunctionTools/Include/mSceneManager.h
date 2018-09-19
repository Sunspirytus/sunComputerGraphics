#ifndef __MSCENEMANAGER_H_
#define __MSCENEMANAGER_H_

#include<vector>
#include<algorithm>
#include"mSuroundBox.h"
#include"mModel.h"
using namespace std;

struct QuadNode
{
	mSuroundBox srBox;
	mSuroundBox smallBox[4];
	vector<mModel*> Models;
	int Depth;
	QuadNode * children[4];
	QuadNode(int depth, float Xmin, float Xmax, float Ymin, float Ymax, float Zmin, float Zmax){
		Depth = depth;
		srBox.CreateBoxFromCornerWorld(Xmin, Xmax, Ymin, Ymax, Zmin, Zmax);
		smallBox[0].CreateBoxFromCornerWorld((Xmin + Xmax) / 2, Xmax, Ymin, Ymax, (Zmin + Zmax) / 2, Zmax);
		smallBox[1].CreateBoxFromCornerWorld(Xmin, (Xmin + Xmax) / 2, Ymin, Ymax, (Zmin + Zmax) / 2, Zmax);
		smallBox[2].CreateBoxFromCornerWorld(Xmin, (Xmin + Xmax) / 2, Ymin, Ymax, Zmin, (Zmin + Zmax) / 2);
		smallBox[3].CreateBoxFromCornerWorld((Xmin + Xmax) / 2, Xmax, Ymin, Ymax, Zmin, (Zmin + Zmax) / 2);
		children[0] = nullptr;
		children[1] = nullptr;
		children[2] = nullptr;
		children[3] = nullptr;
	}
};

class mSceneManager
{
public:
	mSceneManager();
	mSceneManager(int depth, float Xmin, float Xmax, float Ymin, float Ymax, float Zmin, float Zmax);
	~mSceneManager();
	
	vector<mModel*> ModelsNeedRender(PVRTMat4 & VP_Matrix);
	vector<mModel*> ModelsNeedRender2(PVRTMat4 & VP_Matrix);
	vector<mModel*> ModelInScene;
	int Count = 0;

	void addModel(mModel * model);
	void makeQuadTree();
	void Destroy();

private:
	QuadNode * QuadNodeHead;
	int QuadTreeDepth;
	
	vector<mModel*> ModelWaitRender;
	void makeQuadNode(QuadNode * ptr, vector<mModel*> ModelWaitArrange);
	void checkQuadTree(QuadNode * ptr, PVRTMat4 & VP_Matrix);
	void deleteQuadNode(QuadNode* ptr);
};












#endif // !__MSCENEMANAGER_H
