#include "..\Include\mSceneManager.h"

mSceneManager::mSceneManager()
{
}

mSceneManager::~mSceneManager()
{
	
}

void mSceneManager::deleteQuadNode(QuadNode * ptr)
{
	if (ptr == nullptr) return;
	deleteQuadNode(ptr->children[0]);
	deleteQuadNode(ptr->children[1]);
	deleteQuadNode(ptr->children[2]);
	deleteQuadNode(ptr->children[3]);
	delete ptr;
	ptr = nullptr;
}

mSceneManager::mSceneManager(int depth, float Xmin, float Xmax, float Ymin, float Ymax, float Zmin, float Zmax)
{
	this->QuadNodeHead = new QuadNode(1, Xmin, Xmax, Ymin, Ymax, Zmin, Zmax);
	this->QuadTreeDepth = depth;
}

void mSceneManager::addModel(mModel * model)
{
	this->ModelInScene.push_back(model);
}

void mSceneManager::makeQuadTree()
{
	makeQuadNode(this->QuadNodeHead, this->ModelInScene);
}

void mSceneManager::makeQuadNode(QuadNode * ptr, vector<mModel*> ModelWaitArrange)
{
	if (ptr == nullptr) return;
	vector<mModel*> ModelWaitArrangeChild[4];
	while (ModelWaitArrange.size())
	{
		mModel* currentModel = ModelWaitArrange[0];
		bool isInsiede = currentModel->SurrondBox.CenterInsideBoxWorldSpace(ptr->srBox);
		if (isInsiede){
			if (ptr->Depth == this->QuadTreeDepth){
				ptr->Models.push_back(currentModel);
			}
			else{
				for (int m = 0; m < 4; m++){
					if (currentModel->SurrondBox.CenterInsideBoxWorldSpace(ptr->smallBox[m])){
						ModelWaitArrangeChild[m].push_back(currentModel);
						break;
					}
				}
			}
		}
		ModelWaitArrange.erase(ModelWaitArrange.begin());
	}
	if (ptr->Depth == this->QuadTreeDepth){
		ptr->children[0] = nullptr;
		ptr->children[1] = nullptr;
		ptr->children[2] = nullptr;
		ptr->children[3] = nullptr;
	}
	else{
		if (ModelWaitArrangeChild[0].size() == 0) {
			ptr->children[0] = nullptr; 
		}
		if (ModelWaitArrangeChild[1].size() == 0) {
			ptr->children[1] = nullptr; 
		}
		if (ModelWaitArrangeChild[2].size() == 0) {
			ptr->children[2] = nullptr; 
		}
		if (ModelWaitArrangeChild[3].size() == 0){
			ptr->children[3] = nullptr; 
		}
		
		float Xmin = ptr->srBox.pointMin.x;
		float Ymin = ptr->srBox.pointMin.y;
		float Zmin = ptr->srBox.pointMin.z;
		float Xmax = ptr->srBox.pointMax.x;
		float Ymax = ptr->srBox.pointMax.y;
		float Zmax = ptr->srBox.pointMax.z;
		ptr->children[0] = new QuadNode(ptr->Depth + 1, (Xmin + Xmax) / 2, Xmax, Ymin, Ymax, (Zmin + Zmax) / 2, Zmax);
		ptr->children[1] = new QuadNode(ptr->Depth + 1, Xmin, (Xmin + Xmax) / 2, Ymin, Ymax, (Zmin + Zmax) / 2, Zmax);
		ptr->children[2] = new QuadNode(ptr->Depth + 1, Xmin, (Xmin + Xmax) / 2, Ymin, Ymax, Zmin, (Zmin + Zmax) / 2);
		ptr->children[3] = new QuadNode(ptr->Depth + 1, (Xmin + Xmax) / 2, Xmax, Ymin, Ymax, Zmin, (Zmin + Zmax) / 2);
		this->makeQuadNode(ptr->children[0], ModelWaitArrangeChild[0]);
		this->makeQuadNode(ptr->children[1], ModelWaitArrangeChild[1]);
		this->makeQuadNode(ptr->children[2], ModelWaitArrangeChild[2]);
		this->makeQuadNode(ptr->children[3], ModelWaitArrangeChild[3]);
	}
}

vector<mModel*> mSceneManager::ModelsNeedRender(PVRTMat4 & VP_Matrix)
{
	this->Count = 0;
	this->ModelWaitRender.clear();
	this->checkQuadTree(this->QuadNodeHead, VP_Matrix);
	return this->ModelWaitRender;
}

void mSceneManager::checkQuadTree(QuadNode * ptr, PVRTMat4 & VP_Matrix)
{
	if (ptr == nullptr) return;
	if (ptr->srBox.NeedClipFromWorldSpace(VP_Matrix) == false){
		this->Count++;
		if (ptr->Depth == this->QuadTreeDepth){
			for (unsigned int i = 0; i < ptr->Models.size(); i++){
				ptr->Models[i]->needRender = true;
			}
		}
		
		this->checkQuadTree(ptr->children[0], VP_Matrix);
		this->checkQuadTree(ptr->children[1], VP_Matrix);
		this->checkQuadTree(ptr->children[2], VP_Matrix);
		this->checkQuadTree(ptr->children[3], VP_Matrix);
	}
}

void mSceneManager::Destroy()
{
	this->deleteQuadNode(this->QuadNodeHead);
}