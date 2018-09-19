/******************************************************************************

@File         OGLES2PeaceWaterRender.cpp

@Title        Glass

@Version

@Copyright    Copyright (c) Imagination Technologies Limited.

@Platform     Independent

@Description  Demonstrates dynamic reflection and refraction by rendering two
halves of the scene to a single rectangular texture.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES2Tools.h"
#include "..\mFunctionTools\mFunctions.h"
#include <iostream>
#include <limits.h>
#include <vector>
#include <queue>

using namespace std;
/******************************************************************************
Constants
******************************************************************************/
// Camera constants used to generate the projection matrix
const float g_fCamNear = 1.0f;
const float g_fCamFar = 10000.0;
const float g_fCamFOV = PVRT_PI / 3.0f;

enum EVertexAttrib
{
	VERTEX_ARRAY, NORMAL_ARRAY, TANGENT_ARRAY, BINORMAL_ARRAY, TEXCOORD_ARRAY, eNumAttribs
};
const char* g_aszAttribNames[] =
{
	"inVertex", "inNormal", "inTangent", "inBiNormal", "inTexCoords"
};

struct DefaultProgram
{
	enum EUniform{ eMVPMatrix, eMMatrix, eMMatrix_IT, eLightDirModel, eEyePosModel, eTime, eFogColor, eFogDepthRatio, eNumUniforms };
	enum eUniformSampler{ eSmallWaves_NormalTex, eReflectionTex, eRefractionTex, eSkybox_Tex, eNumUniformSamplers };
	GLuint uiId;
	GLuint auiLoc[eNumUniforms];
};

struct BlinnPhongProgram
{
	enum EUniform{ eMVPMatrix, eLightDirModel, eEyePosModel, eDiffuseColor, eNumUniforms };
	enum eUniformSampler{ eDiffuseTex, eNumUniformSamplers };
	GLuint uiId;
	GLuint auiLoc[eNumUniforms];
};

struct SkyboxProgram{
	enum EUniform{ eMMatrix, eMVPMatrix, ebDrawFog, eFogColor, eFogHeight, eFogHeightRatio, eNumUniforms };
	enum eUniformSampler{ eSkyboxTex, eNumUniformSamplers };
	GLuint uiId;
	GLuint auiLoc[eNumUniforms];
};


// Shader uniforms

/******************************************************************************
Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[] = "DefaultFragShader.fsh";
const char c_szFragShaderBinFile[] = "DefaultFragShader.fsc";
const char c_szVertShaderSrcFile[] = "DefaultVertShader.vsh";
const char c_szVertShaderBinFile[] = "DefaultVertShader.vsc";

const char c_szSkyboxVertShaderSrcFile[] = "SkyboxVertShader.vsh";
const char c_szSkyboxVertShaderBinFile[] = "SkyboxVertShader.vsc";
const char c_szSkyboxFragShaderSrcFile[] = "SkyboxFragShader.fsh";
const char c_szSkyboxFragShaderBinFile[] = "SkyboxFragShader.fsc";

const char c_szBPFragShaderSrcFile[] = "BlinnPhongFragShader.fsh";
const char c_szBPFragShaderBinFile[] = "BlinnPhongFragShader.fsc";
const char c_szBPVertShaderSrcFile[] = "BlinnPhongVertShader.vsh";
const char c_szBPVertShaderBinFile[] = "BlinnPhongVertShader.vsc";

// PVR texture files
const char c_szSmallWaves_N_TexFile[] = "SmallWaves_N_Tex.pvr";
const char c_szSkybox1_TexFile[] = "Skybox1_Tex.pvr";
const char c_szSkybox2_TexFile[] = "Skybox2_Tex.pvr";

// POD scene files
const char c_szBallFile[] = "Ball.pod";
const char c_szCube_testFile[] = "cube_test.pod";
const int WaterFileScale = 100;
const char c_szWaterPlaneFile[] = "WaterPlane100x100.pod";


/*!****************************************************************************
Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2PeaceWaterRender : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	// 3D Models
	CPVRTModelPOD m_BallPOD;
	CPVRTModelPOD m_CubePOD;
	CPVRTModelPOD m_WaterPlanePOD;
	mModel m_Ball;
	mModel m_Cube;
	mModel m_WaterPlane;
	vector<mModel> m_WaterGroup;
	queue<mModel*> m_WaterRenderQueue;
	vector<mModel*> m_WaterGroupFromSceneManager;

	// Projection, view and model matrices
	float m_RotateAngleX, m_RotateAngleY, m_RotateAngleZ;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiDefaultVertShader;
	GLuint m_uiDefaultFragShader;
	GLuint m_uiBPVertShader;
	GLuint m_uiBPFragShader;
	GLuint m_uiSkyboxVertShader;
	GLuint m_uiSkyboxFragShader;

	GLuint m_uiSmallWaves_N_Tex;
	GLuint m_uiSkybox1_Tex;
	GLuint m_uiSkybox2_Tex;

	GLuint m_puiSkyboxVbo;

	// Skybox
	GLfloat* m_SkyboxVertices;
	GLfloat* m_SkyboxTexCoords;
	PVRTVec4 m_FogColor;
	float m_FogHeightRatio;

	//FBOs and renderTex
	GLint m_iOriginalFBO;
	GLuint m_auiReflectFBO;
	GLuint m_uiReflectRenderTex;
	GLuint m_auiReflectDepthBuffer;
	GLuint m_auiRefractFBO;
	GLuint m_uiRefractRenderTex;
	GLuint m_auiRefractDepthBuffer;


	//WorldSpace
	PVRTVec4 m_globalLightDir;

	// Group shader programs and their uniform locations together
	DefaultProgram m_DefaultProgram;
	BlinnPhongProgram m_BlinnPhongProgram;
	SkyboxProgram m_SkyboxProgram;

	Camera MainCamera;
	Camera ReflectionCamera;
	Camera WatchCameraTTP;
	bool TTPmode;
	bool FrustumClipOn;
	mSceneManager m_SceneManager;

	// Current time in milliseconds
	float m_ulTime;
	unsigned long m_ulPreviousTime, m_ulCurrentTime;
	float m_fElapsedTimeInSecs, m_fDeltaTime, m_fFrame, m_fCount;
	unsigned int m_uiFrameCount;
	vector<unsigned int> m_uiFPS;
	int m_uiFpsOut;

	float height, w;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

private:
	void LoadVbos();

	bool LoadDefaultShader(CPVRTString* pErrorStr);
	bool LoadBlinnPhongShader(CPVRTString* pErrorStr);
	bool LoadSkyboxShader(CPVRTString* pErrorStr);


	bool LoadModels(CPVRTString* pErrorStr);
	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);

	void ShowFPS();

	void RenderReflectionTex(Camera camera);
	void RenderRefractionTex(Camera camera);

	template<class T>
	void DrawMesh(int i32NodeIndex, CPVRTModelPOD* pod, GLuint** ppuiVbos, GLuint** ppuiIbos, T & i32Attributes);

	void DrawBall(Camera & camera, PVRTVec3 position, PVRTVec3 diffuseColor);
	void DrawCube(Camera & camera);
	void DrawWaterPlanes(Camera & camera);
	void DrawSkybox(Camera & camera, int bDrawFog);
};

/*!****************************************************************************
@Function		LoadModels
@Output		pErrorStr		A string describing the error on failure
@Return		bool			true if no error occured
@Description	Loads the Models required for this training course
******************************************************************************/
bool OGLES2PeaceWaterRender::LoadModels(CPVRTString* const pErrorStr){
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the ball
	if (m_BallPOD.ReadFromFile(c_szBallFile) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Couldn't load the .pod file\n";
		return false;
	}

	//Load the cube
	if (m_CubePOD.ReadFromFile(c_szCube_testFile) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Couldn't load the Cube_test .pod file\n";
		return false;
	}

	//Load the WaterPlane
	if (m_WaterPlanePOD.ReadFromFile(c_szWaterPlaneFile) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Couldn't load the WaterPlane.pod file\n";
		return false;
	}

	return true;
}

/*!****************************************************************************
@Function		LoadTextures
@Output		pErrorStr		A string describing the error on failure
@Return		bool			true if no error occured
@Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES2PeaceWaterRender::LoadTextures(CPVRTString* const pErrorStr)
{
	/*if (PVRTTextureLoadFromPVR(c_szHairDiffWHiteTexFile, &m_uiHairDiffWhiteTex) != PVR_SUCCESS){
	*pErrorStr = "ERROR: Failed to load Hair Diffuse Tex.";
	return false;
	}*/
	if (PVRTTextureLoadFromPVR(c_szSmallWaves_N_TexFile, &m_uiSmallWaves_N_Tex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load SmallWaves_Normal Tex.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (PVRTTextureLoadFromPVR(c_szSkybox1_TexFile, &m_uiSkybox1_Tex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load Skybox Tex";
		return false;
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (PVRTTextureLoadFromPVR(c_szSkybox2_TexFile, &m_uiSkybox2_Tex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load Skybox Tex";
		return false;
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	//Reflection, Refraction Render Texture
	glGenTextures(1, &m_uiReflectRenderTex);
	glBindTexture(GL_TEXTURE_2D, m_uiReflectRenderTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PVRShellGet(prefWidth), PVRShellGet(prefHeight), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &m_uiRefractRenderTex);
	glBindTexture(GL_TEXTURE_2D, m_uiRefractRenderTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PVRShellGet(prefWidth), PVRShellGet(prefHeight), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return true;
}

bool OGLES2PeaceWaterRender::LoadSkyboxShader(CPVRTString* pErrorStr)
{
	const char* g_aszUniformNames[] =
	{
		"ModelMatrix", "MVPMatrix", "bDrawFog", "FogColor", "FogHeight", "FogHeightRatio"
	};
	const char* g_aszUniformSamplerNames[] =
	{
		"Skybox_Tex"
	};

	/*
	Load and compile the shaders from files.
	Binary shaders are tried first, source shaders
	are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
		c_szSkyboxVertShaderBinFile, c_szSkyboxVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiSkyboxVertShader,
		pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
		c_szSkyboxFragShaderBinFile, c_szSkyboxFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiSkyboxFragShader,
		pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
	Set up and link the shader program
	*/
	if (PVRTCreateProgram(&m_SkyboxProgram.uiId, m_uiSkyboxVertShader, m_uiSkyboxFragShader, g_aszAttribNames, 1,
		pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < m_SkyboxProgram.eNumUniforms; ++i)
	{
		m_SkyboxProgram.auiLoc[i] = glGetUniformLocation(m_SkyboxProgram.uiId, g_aszUniformNames[i]);
	}

	for (int i = 0; i < m_SkyboxProgram.eNumUniformSamplers; i++){
		glUniform1i(glGetUniformLocation(m_SkyboxProgram.uiId, g_aszUniformSamplerNames[i]), i);
	}

	return true;
}

bool OGLES2PeaceWaterRender::LoadBlinnPhongShader(CPVRTString* pErrorStr)
{
	const char* g_aszAttribNames[] =
	{
		"inVertex", "inNormal", "inTangent", "inBiNormal", "inTexCoords"
	};
	const char* g_aszUniformNames[] =
	{
		"MVPMatrix", "LightDirModel", "EyePosModel", "diffuseColor"
	};
	const char* g_aszUniformSamplerNames[] =
	{
		"diffuseTex"
	};

	/*
	Load and compile the shaders from files.
	Binary shaders are tried first, source shaders
	are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
		c_szBPVertShaderBinFile, c_szBPVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiBPVertShader,
		pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
		c_szBPFragShaderBinFile, c_szBPFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiBPFragShader,
		pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
	Set up and link the shader program
	*/
	if (PVRTCreateProgram(&m_BlinnPhongProgram.uiId, m_uiBPVertShader, m_uiBPFragShader, g_aszAttribNames, eNumAttribs,
		pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < m_BlinnPhongProgram.eNumUniforms; ++i)
	{
		m_BlinnPhongProgram.auiLoc[i] = glGetUniformLocation(m_BlinnPhongProgram.uiId, g_aszUniformNames[i]);
	}

	for (int i = 0; i < m_BlinnPhongProgram.eNumUniformSamplers; i++){
		glUniform1i(glGetUniformLocation(m_BlinnPhongProgram.uiId, g_aszUniformSamplerNames[i]), i);
	}

	return true;
}

bool OGLES2PeaceWaterRender::LoadDefaultShader(CPVRTString* pErrorStr)
{
	const char* g_aszUniformNames[] =
	{
		"MVPMatrix", "MMatrix", "MMatrix_IT", "LightDirModel", "EyePosModel", "_Time", "FogColor", "FogDepthRatio"
	};
	const char* g_aszUniformSamplerNames[] =
	{
		"SmallWaves_NormalTex", "ReflectionTex", "RefractionTex", "Skybox_Tex"
	};
	/*
	Load and compile the shaders from files.
	Binary shaders are tried first, source shaders
	are used as fallback.
	*/
	if (PVRTShaderLoadFromFile(
		c_szVertShaderBinFile, c_szVertShaderSrcFile, GL_VERTEX_SHADER, GL_SGX_BINARY_IMG, &m_uiDefaultVertShader,
		pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	if (PVRTShaderLoadFromFile(
		c_szFragShaderBinFile, c_szFragShaderSrcFile, GL_FRAGMENT_SHADER, GL_SGX_BINARY_IMG, &m_uiDefaultFragShader,
		pErrorStr) != PVR_SUCCESS)
	{
		return false;
	}

	/*
	Set up and link the shader program
	*/
	if (PVRTCreateProgram(&m_DefaultProgram.uiId, m_uiDefaultVertShader, m_uiDefaultFragShader, g_aszAttribNames, eNumAttribs,
		pErrorStr) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, pErrorStr->c_str());
		return false;
	}

	// Store the location of uniforms for later use
	for (int i = 0; i < m_DefaultProgram.eNumUniforms; ++i)
	{
		m_DefaultProgram.auiLoc[i] = glGetUniformLocation(m_DefaultProgram.uiId, g_aszUniformNames[i]);
	}
	for (int i = 0; i < m_DefaultProgram.eNumUniformSamplers; i++){
		glUniform1i(glGetUniformLocation(m_DefaultProgram.uiId, g_aszUniformSamplerNames[i]), i);
	}
	return true;
}


/*!****************************************************************************
@Function		LoadShaders
@Output		pErrorStr		A string describing the error on failure
@Return		bool			true if no error occured
@Description	Loads and compiles the shaders and links the shader programs
required for this training course
******************************************************************************/
bool OGLES2PeaceWaterRender::LoadShaders(CPVRTString* pErrorStr)
{
	if (!LoadDefaultShader(pErrorStr)){
		return false;
	}
	if (!LoadBlinnPhongShader(pErrorStr)){
		return false;
	}
	if (!LoadSkyboxShader(pErrorStr)){
		return false;
	}
	return true;
}

/*!****************************************************************************
@Function		LoadVbos
@Description	Loads the mesh data required for this training course into
vertex buffer objects
******************************************************************************/
void OGLES2PeaceWaterRender::LoadVbos()
{
	m_Ball.LoadVBO();
	m_Cube.LoadVBO();
	m_WaterPlane.LoadVBO();

	glGenBuffers(1, &m_puiSkyboxVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_puiSkyboxVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 24, m_SkyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*!****************************************************************************
@Function		InitApplication
@Return		bool		true if no error occurred
@Description	Code in InitApplication() will be called by PVRShell once per
run, before the rendering context is created.
Used to initialize variables that are not dependant on it
(e.g. external modules, loading meshes, etc.)
If the rendering context is lost, InitApplication() will
not be called again.
******************************************************************************/
bool OGLES2PeaceWaterRender::InitApplication()
{
	CPVRTString pErrorStr;
	if (!LoadModels(&pErrorStr)){
		PVRShellSet(prefExitMessage, pErrorStr.c_str());
		return false;
	}
	PVRShellSet(prefAASamples, 8);
	PVRShellSet(prefAppName, "WaterPeace");
	PVRShellSet(prefSwapInterval, 0);

	m_Ball.SetPOD(&m_BallPOD);
	m_Cube.SetPOD(&m_CubePOD);
	m_WaterPlane.SetPOD(&m_WaterPlanePOD);

	m_ulTime = 0.0;

	m_ulCurrentTime = PVRShellGetTime();
	m_ulPreviousTime = m_ulCurrentTime;
	m_fCount = 0;
	m_uiFrameCount = 0;
	m_uiFpsOut = 0;

	m_RotateAngleX = 0;
	m_RotateAngleY = 0;
	m_RotateAngleZ = 0;

	m_FogColor = PVRTVec4(0.431f, 0.373f, 0.333f, 1.0f);
	m_FogHeightRatio = 1.0f / 500.0f;

	height = 1.0;
	w = 3.0;

	TTPmode = false;
	FrustumClipOn = true;

	m_SceneManager = mSceneManager(4, -2250, 2250, -500, 500, -2250, 2250);

	return true;
}

/*!****************************************************************************
@Function		QuitApplication
@Return		bool		true if no error occurred
@Description	Code in QuitApplication() will be called by PVRShell once per
run, just before exiting the program.
If the rendering context is lost, QuitApplication() will
not be called.
******************************************************************************/
bool OGLES2PeaceWaterRender::QuitApplication()
{
	// Free the memory allocated for the scene
	m_Ball.Destroy();
	m_Cube.Destroy();
	m_WaterPlane.Destroy();
	m_SceneManager.Destroy();
	delete[] m_SkyboxVertices;
	delete[] m_SkyboxTexCoords;
	return true;
}

/*!****************************************************************************
@Function		InitView
@Return		bool		true if no error occurred
@Description	Code in InitView() will be called by PVRShell upon
initialization or after a change in the rendering context.
Used to initialize variables that are dependant on the rendering
context (e.g. textures, vertex buffers, etc.)
******************************************************************************/
bool OGLES2PeaceWaterRender::InitView()
{
	CPVRTString ErrorStr;

	// Create the skybox
	PVRTCreateSkybox(6000, true, 512, &m_SkyboxVertices, &m_SkyboxTexCoords);
	/*
	Initialize VBO data
	*/
	LoadVbos();

	/*
	Load textures
	*/
	if (!LoadTextures(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	/*
	Load and compile the shaders & link programs
	*/
	if (!LoadShaders(&ErrorStr))
	{
		PVRShellSet(prefExitMessage, ErrorStr.c_str());
		return false;
	}

	// Is the screen rotated?
	bool bRotate = PVRShellGet(prefIsRotated) && PVRShellGet(prefFullScreen);
	/*
	Initialize Print3D
	*/
	if (m_Print3D.SetTextures(0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), bRotate) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Cannot initialise Print3D\n");
		return false;
	}

	/*
	Handle FrameBuffer
	*/
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_iOriginalFBO);

	glGenFramebuffers(1, &m_auiReflectFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiReflectFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiReflectRenderTex, 0);

	glGenRenderbuffers(1, &m_auiReflectDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_auiReflectDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_auiReflectDepthBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Reflection Frame buffer did not set up correctly\n");
		return false;
	}

	glGenFramebuffers(1, &m_auiRefractFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiRefractFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiRefractRenderTex, 0);

	glGenRenderbuffers(1, &m_auiRefractDepthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_auiRefractDepthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_auiRefractDepthBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		PVRShellSet(prefExitMessage, "ERROR: Reflection Frame buffer did not set up correctly\n");
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);


	//Prepare transform and camera
	m_WaterPlane.CreateSuroundBox();
	m_WaterPlane.SetScale(1.0, 1.0, 1.0);
	if (WaterFileScale == 300){
		for (int i = -1; i <= 1; i++){
			for (int j = -1; j <= 1; j++){
				m_WaterPlane.SetPosition(i * 1500.0f, 0.0f, j * 1500.0f);
				m_WaterGroup.push_back(m_WaterPlane);
			}
		}
	}
	else if (WaterFileScale == 225){
		for (int i = -2; i <= 2; i++){
			for (int j = -2; j <= 2; j++){
				if (i < 0 && j < 0){
					m_WaterPlane.SetPosition(i * 1125.0f + 562.5f, 0.0f, j * 1125.0f + 562.5f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i < 0 && j > 0){
					m_WaterPlane.SetPosition(i * 1125.0f + 562.5f, 0.0f, j * 1125.0f - 562.5f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j < 0){
					m_WaterPlane.SetPosition(i * 1125.0f - 562.5f, 0.0f, j * 1125.0f + 562.5f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j > 0){
					m_WaterPlane.SetPosition(i * 1125.0f - 562.5f, 0.0f, j * 1125.0f - 562.5f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
			}
		}
	}
	else if (WaterFileScale == 180){
		for (int i = -2; i <= 2; i++){
			for (int j = -2; j <= 2; j++){
				m_WaterPlane.SetPosition(i * 900.0f, 0.0f, j * 900.0f);
				m_WaterGroup.push_back(m_WaterPlane);
			}
		}
	}
	else if (WaterFileScale == 150){
		for (int i = -3; i <= 3; i++){
			for (int j = -3; j <= 3; j++){
				if (i < 0 && j < 0){
					m_WaterPlane.SetPosition(i * 750.0f + 375.0f, 0.0f, j * 750.0f + 375.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i < 0 && j > 0){
					m_WaterPlane.SetPosition(i * 750.0f + 375.0f, 0.0f, j * 750.0f - 375.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j < 0){
					m_WaterPlane.SetPosition(i * 750.0f - 375.0f, 0.0f, j * 750.0f + 375.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j > 0){
					m_WaterPlane.SetPosition(i * 750.0f - 375.0f, 0.0f, j * 750.0f - 375.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
			}
		}
	}
	else if (WaterFileScale == 130){
		for (int i = -3; i <= 3; i++){
			for (int j = -3; j <= 3; j++){
				m_WaterPlane.SetPosition(i * 650.0f, 0.0f, j * 650.0f);
				m_WaterGroup.push_back(m_WaterPlane);
			}
		}
	}
	else if (WaterFileScale == 120){
		for (int i = -4; i <= 4; i++){
			for (int j = -4; j <= 4; j++){
				if (i < 0 && j < 0){
					m_WaterPlane.SetPosition(i * 600.0f + 300.0f, 0.0f, j * 600.0f + 300.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i < 0 && j > 0){
					m_WaterPlane.SetPosition(i * 600.0f + 300.0f, 0.0f, j * 600.0f - 300.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j < 0){
					m_WaterPlane.SetPosition(i * 600.0f - 300.0f, 0.0f, j * 600.0f + 300.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j > 0){
					m_WaterPlane.SetPosition(i * 600.0f - 300.0f, 0.0f, j * 600.0f - 300.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
			}
		}
	}
	else if (WaterFileScale == 110){
		for (int i = -4; i <= 4; i++){
			for (int j = -4; j <= 4; j++){
				if (i < 0 && j < 0){
					m_WaterPlane.SetPosition(i * 550.0f + 275.0f, 0.0f, j * 550.0f + 275.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i < 0 && j > 0){
					m_WaterPlane.SetPosition(i * 550.0f + 275.0f, 0.0f, j * 550.0f - 275.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j < 0){
					m_WaterPlane.SetPosition(i * 550.0f - 275.0f, 0.0f, j * 550.0f + 275.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j > 0){
					m_WaterPlane.SetPosition(i * 550.0f - 275.0f, 0.0f, j * 550.0f - 275.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
			}
		}
	}
	else if (WaterFileScale == 100){
		for (int i = -4; i <= 4; i++){
			for (int j = -4; j <= 4; j++){
				m_WaterPlane.SetPosition(i * 500.0f, 0.0f, j * 500.0f);
				m_WaterGroup.push_back(m_WaterPlane);
			}
		}
	}
	else if (WaterFileScale == 75){
		for (int i = -6; i <= 6; i++){
			for (int j = -6; j <= 6; j++){
				if (i < 0 && j < 0){
					m_WaterPlane.SetPosition(i * 375.0f + 187.5f, 0.0f, j * 375.0f + 187.5f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i < 0 && j > 0){
					m_WaterPlane.SetPosition(i * 375.0f + 187.5f, 0.0f, j * 375.0f - 187.5f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j < 0){
					m_WaterPlane.SetPosition(i * 375.0f - 187.5f, 0.0f, j * 375.0f + 187.5f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j > 0){
					m_WaterPlane.SetPosition(i * 375.0f - 187.5f, 0.0f, j * 375.0f - 187.5f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
			}
		}
	}
	else if (WaterFileScale == 50){
		for (int i = -9; i <= 9; i++){
			for (int j = -9; j <= 9; j++){
				if (i < 0 && j < 0){
					m_WaterPlane.SetPosition(i * 250.0f + 125.0f, 0.0f, j * 250.0f + 125.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i < 0 && j > 0){
					m_WaterPlane.SetPosition(i * 250.0f + 125.0f, 0.0f, j * 250.0f - 125.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j < 0){
					m_WaterPlane.SetPosition(i * 250.0f - 125.0f, 0.0f, j * 250.0f + 125.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
				else if (i > 0 && j > 0){
					m_WaterPlane.SetPosition(i * 250.0f - 125.0f, 0.0f, j * 250.0f - 125.0f);
					m_WaterGroup.push_back(m_WaterPlane);
				}
			}
		}
	}
	else if (WaterFileScale == 20){
		for (int i = -22; i <= 22; i++){
			for (int j = -22; j <= 22; j++){
				m_WaterPlane.SetPosition(i * 100.0f, 0.0, j * 100.0f);
				m_WaterGroup.push_back(m_WaterPlane);
			}
		}
	}
	else{
		PVRShellSet(prefExitMessage, "ERROR: File Error");
		return false;
	}

	for (unsigned int i = 0; i < m_WaterGroup.size(); ++i){
		m_SceneManager.addModel(&m_WaterGroup[i]);
	}
	m_SceneManager.makeQuadTree();

	m_Ball.SetScale(50.0, 50.0, 50.0);

	m_globalLightDir = PVRTVec4(0, 0, 0, 0) - PVRTVec4(0, -1, -5, 0);

	MainCamera = Camera(PVRTVec3(0.0, 100.0f, 0.0),
		PVRTVec3(0.0, 0.0, 0.0),
		g_fCamFOV,
		(float)PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight),
		g_fCamNear,
		g_fCamFar,
		PVRTMat4::eClipspace::OGL,
		bRotate);

	ReflectionCamera = Camera(PVRTVec3(0.0, 100.0, 0.0),
		PVRTVec3(0.0, 0.0, 0.0),
		g_fCamFOV,
		(float)PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight),
		g_fCamNear,
		g_fCamFar,
		PVRTMat4::eClipspace::OGL,
		bRotate);

	WatchCameraTTP = Camera(PVRTVec3(0, 5000, 0),
		PVRTVec3(-90.0, 0.0, 0.0),
		g_fCamFOV,
		(float)PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight),
		g_fCamNear,
		g_fCamFar,
		PVRTMat4::eClipspace::OGL,
		bRotate);

	/*
	Set OpenGL ES render states needed for this training course
	*/

	glClearColor(0.6f, 0.8f, 1.0f, 0.0f);

	return true;
}

/*!****************************************************************************
@Function		ReleaseView
@Return		bool		true if no error occured
@Description	Code in ReleaseView() will be called by PVRShell when the
application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2PeaceWaterRender::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiSmallWaves_N_Tex);

	// Delete program objects
	glDeleteProgram(m_DefaultProgram.uiId);
	glDeleteProgram(m_BlinnPhongProgram.uiId);
	glDeleteProgram(m_SkyboxProgram.uiId);

	// Delete shader objects
	glDeleteShader(m_uiDefaultVertShader);
	glDeleteShader(m_uiDefaultFragShader);
	glDeleteShader(m_uiBPVertShader);
	glDeleteShader(m_uiBPFragShader);
	glDeleteShader(m_uiSkyboxVertShader);
	glDeleteShader(m_uiSkyboxFragShader);


	// Delete buffer objects
	m_Ball.DeleteVBOs();
	m_Cube.DeleteVBOs();
	m_WaterPlane.DeleteVBOs();

	// Delete renderbuffers
	glDeleteRenderbuffers(1, &m_auiReflectDepthBuffer);

	// Delete framebuffers
	glDeleteFramebuffers(1, &m_auiReflectFBO);

	// Release Print3D Textures
	m_Print3D.ReleaseTextures();

	return true;
}

/*!****************************************************************************
@Function		RenderScene
@Return		bool		true if no error occured
@Description	Main rendering loop function of the program. The shell will
call this function every frame.
eglSwapBuffers() will be performed by PVRShell automatically.
PVRShell will also manage important OS events.
Will also manage relevent OS events. The user has access to
these events through an abstraction layer provided by PVRShell.
******************************************************************************/
bool OGLES2PeaceWaterRender::RenderScene()
{
	ShowFPS();
	if (PVRShellIsKeyPressed(PVRShellKeyNameUP)){
		//height += 0.5;
		//m_RotateAngleX += 10.0f;
		TTPmode = true;
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN)){
		//height -= 0.5;
		//m_RotateAngleX += -10.0f;
		TTPmode = false;
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT)){
		//w += 1.0;
		//m_RotateAngleY += 10.0f;
		FrustumClipOn = true;
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT)){
		//w -= 1.0;
		//m_RotateAngleY += -10.0f;
		FrustumClipOn = false;
	}

	if (!TTPmode){
		m_RotateAngleY -= 0.1f;
		m_FogHeightRatio = 1.0f / 500.0f;
		MainCamera.setEulerAngle(m_RotateAngleX, m_RotateAngleY, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Clear the color

		ReflectionCamera.setPosition(MainCamera.getPosition().x, -MainCamera.getPosition().y, MainCamera.getPosition().z);
		ReflectionCamera.setEulerAngle(-MainCamera.getEulerAngle().x, MainCamera.getEulerAngle().y, MainCamera.getEulerAngle().z);
		RenderReflectionTex(ReflectionCamera);
		RenderRefractionTex(MainCamera);

		glEnable(GL_DEPTH_TEST);

		DrawSkybox(MainCamera, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawSkybox(MainCamera, 1);

		if (FrustumClipOn){
			for (unsigned int i = 0; i < m_SceneManager.ModelInScene.size(); i++){
				PVRTMat4 mModel, mModelView, mMVP;
				mModel = m_SceneManager.ModelInScene[i]->GetModelMatrix();
				mMVP = MainCamera.getVPMatrix() * mModel;
				if (!m_SceneManager.ModelInScene[i]->NeedClip(mMVP)){
					m_SceneManager.ModelInScene[i]->needRender = true;
				}
			}
		}
		else{
			MainCamera.setPosition(MainCamera.getPosition() + MainCamera.getForward() * (-1200.0));
			m_WaterGroupFromSceneManager = m_SceneManager.ModelsNeedRender(MainCamera.getVPMatrix());
			MainCamera.setPosition(MainCamera.getPosition() + MainCamera.getForward() * (1200.0));
		}

		for (unsigned int i = 0; i < m_SceneManager.ModelInScene.size(); i++){
			if (m_SceneManager.ModelInScene[i]->needRender){
				m_WaterRenderQueue.push(m_SceneManager.ModelInScene[i]);
			}
		}

		m_Print3D.Print3D(0.0, 15.0, 1.0, PVRTRGBA(255, 255, 255, 255), "RenderCount:%i", m_WaterRenderQueue.size());
		m_Print3D.Print3D(0.0, 20.0, 1.0, PVRTRGBA(255, 255, 255, 255), "CompareCountEachFor:%i", m_WaterGroup.size());
		m_Print3D.Print3D(0.0, 25.0, 1.0, PVRTRGBA(255, 255, 255, 255), "CompareCountQuadTree:%i", m_SceneManager.Count);

		DrawWaterPlanes(MainCamera);

		glDisable(GL_DEPTH_TEST);

		if (FrustumClipOn){
			m_Print3D.Print3D(0.0, 10.0, 1.0, PVRTRGBA(255, 255, 255, 255), "ForEachClip");
		}
		else{
			m_Print3D.Print3D(0.0, 10.0, 1.0, PVRTRGBA(255, 255, 255, 255), "QuadTree");
		}

	}
	else{
		m_RotateAngleY -= 0.5f;
		m_FogHeightRatio = 0.0;
		MainCamera.setEulerAngle(m_RotateAngleX, m_RotateAngleY, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Clear the color

		ReflectionCamera.setPosition(WatchCameraTTP.getPosition().x, -WatchCameraTTP.getPosition().y, WatchCameraTTP.getPosition().z);
		ReflectionCamera.setEulerAngle(-WatchCameraTTP.getEulerAngle().x, WatchCameraTTP.getEulerAngle().y, WatchCameraTTP.getEulerAngle().z);
		RenderReflectionTex(ReflectionCamera);

		glEnable(GL_DEPTH_TEST);

		DrawSkybox(WatchCameraTTP, 0);

		GLubyte * tempPixelsBuffer = (GLubyte*)malloc(PVRShellGet(prefWidth) * PVRShellGet(prefHeight) * 4);
		glReadPixels(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight), GL_RGBA, GL_UNSIGNED_BYTE, tempPixelsBuffer);
		glBindTexture(GL_TEXTURE_2D, m_uiRefractRenderTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PVRShellGet(prefWidth), PVRShellGet(prefHeight), 0, GL_RGBA, GL_UNSIGNED_BYTE, tempPixelsBuffer);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawSkybox(WatchCameraTTP, 1);
		DrawBall(WatchCameraTTP, PVRTVec3(0, 100, 0), PVRTVec3(0.0, 0.0, 0.0));

		DrawBall(WatchCameraTTP, PVRTMat4::RotationY(-m_RotateAngleY / 180.0f * PVRT_PI) * PVRTVec4(0.0f, 0.0f, 1.0f, 1.0f) * 500.0f + PVRTVec4(0.0f, 100.0f, 0.0f, 1.0f), PVRTVec3(1.0f, 1.0f, 0.0f));
		DrawBall(WatchCameraTTP, MainCamera.getPosition() + MainCamera.getForward() * (1000.0f) + PVRTVec4(0.0f, 100.0f, 0.0f, 1.0f), PVRTVec3(1.0f, 0.0f, 0.0f));

		if (FrustumClipOn){
			for (unsigned int i = 0; i < m_WaterGroup.size(); i++){
				PVRTMat4 mModel, mModelView, mMVP;
				mModel = m_WaterGroup[i].GetModelMatrix();
				mMVP = MainCamera.getVPMatrix() * mModel;
				if (!m_WaterGroup[i].NeedClip(mMVP)){
					m_WaterRenderQueue.push(&m_WaterGroup[i]);
				}
			}
		}
		else{
			MainCamera.setPosition(MainCamera.getPosition() + MainCamera.getForward() * (-100.0f));
			m_WaterGroupFromSceneManager = m_SceneManager.ModelsNeedRender(MainCamera.getVPMatrix());
			for (unsigned int i = 0; i < m_SceneManager.ModelInScene.size(); i++){
				if (m_SceneManager.ModelInScene[i]->needRender){
					m_WaterRenderQueue.push(m_SceneManager.ModelInScene[i]);
				}
			}
			MainCamera.setPosition(MainCamera.getPosition() + MainCamera.getForward() * (100.0));
		}
		m_Print3D.Print3D(0.0, 15.0, 1.0, PVRTRGBA(255, 255, 255, 255), "RenderCount:%i", m_WaterRenderQueue.size());

		DrawWaterPlanes(WatchCameraTTP);

		//glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		if (FrustumClipOn){
			m_Print3D.Print3D(0.0, 10.0, 1.0, PVRTRGBA(255, 255, 255, 255), "ForEachClip");
		}
		else{
			m_Print3D.Print3D(0.0, 10.0, 1.0, PVRTRGBA(255, 255, 255, 255), "QuadTree");
		}
	}

	for (unsigned int i = 0; i < m_SceneManager.ModelInScene.size(); i++){
		m_SceneManager.ModelInScene[i]->needRender = false;
	}
	m_Print3D.Flush();
	return true;
}

/*!****************************************************************************
@Function		UpdateScene
@Description	Moves the scene.
******************************************************************************/
void OGLES2PeaceWaterRender::ShowFPS()
{
	m_uiFrameCount++;

	m_ulPreviousTime = m_ulCurrentTime;
	m_ulCurrentTime = PVRShellGetTime();

	m_fElapsedTimeInSecs = m_ulCurrentTime * 0.001f;
	m_fDeltaTime = ((float)(m_ulCurrentTime - m_ulPreviousTime))*0.001f;

	m_fCount += m_fDeltaTime;

	if (m_fCount >= 1.0f)			// Update FPS once a second
	{
		m_uiFPS.push_back(m_uiFrameCount);
		m_uiFrameCount = 0;
		m_fCount = 0;
	}

	if (m_uiFPS.size() == 30){
		m_uiFpsOut = 0;
		for (unsigned int i = 0; i < m_uiFPS.size(); i++){
			m_uiFpsOut += m_uiFPS[i];
		}
		m_uiFpsOut /= m_uiFPS.size();

		m_uiFPS.erase(m_uiFPS.begin());
	}

	m_Print3D.Print3D(0.0, 0.0, 1.0, PVRTRGBA(255, 255, 255, 255), "FPS: %2i", m_uiFPS[m_uiFPS.size() - 1]);
	m_Print3D.Print3D(0.0, 5.0, 1.0, PVRTRGBA(255, 255, 255, 255), "FPSAvg: %2i", m_uiFpsOut);

}

/*!****************************************************************************
@Function		DrawMesh
@Input			i32NodeIndex		Node index of the mesh to draw
pod					POD containing the node to draw
ppuiVbos			VBO to bind to
ppuiIbos			IBO to bind to
i32NumAttributes	Number of vertex attributes to activate
@Description	Draws a SPODMesh after the model view matrix has been set and
the meterial prepared.
******************************************************************************/
template<class T>
void OGLES2PeaceWaterRender::DrawMesh(int i32NodeIndex, CPVRTModelPOD* pod, GLuint** ppuiVbos, GLuint** ppuiIbos, T & i32Attributes)
{
	int i32MeshIndex = pod->pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &pod->pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, (*ppuiVbos)[i32MeshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*ppuiIbos)[i32MeshIndex]);

	int NumAttribute = sizeof(i32Attributes) / sizeof(i32Attributes[0]);

	// Enable the vertex attribute arrays
	for (int i = 0; i < NumAttribute; ++i) { glEnableVertexAttribArray(i); }

	// Set the vertex attribute offsets
	for (int i = 0; i < NumAttribute; i++){
		switch (i)
		{
		case VERTEX_ARRAY: glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData); break;
		case NORMAL_ARRAY: glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData); break;
		case TANGENT_ARRAY: glVertexAttribPointer(TANGENT_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sTangents.nStride, pMesh->sTangents.pData); break;
		case BINORMAL_ARRAY: glVertexAttribPointer(BINORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sBinormals.nStride, pMesh->sBinormals.pData); break;
		case TEXCOORD_ARRAY: glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData); break;
		default:
			break;
		}
	}
	/*
	The geometry can be exported in 4 ways:
	- Indexed Triangle list
	- Non-Indexed Triangle list
	- Indexed Triangle strips
	- Non-Indexed Triangle strips
	*/
	if (pMesh->nNumStrips == 0)
	{
		if ((*ppuiIbos)[i32MeshIndex])
		{
			// Indexed Triangle list
			glDrawElements(GL_TRIANGLES, pMesh->nNumFaces * 3, GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			// Non-Indexed Triangle list
			glDrawArrays(GL_TRIANGLES, 0, pMesh->nNumFaces * 3);
		}
	}
	else
	{
		for (int i = 0; i < (int)pMesh->nNumStrips; ++i)
		{
			int offset = 0;
			if ((*ppuiIbos)[i32MeshIndex])
			{
				// Indexed Triangle strips
				glDrawElements(GL_TRIANGLE_STRIP, pMesh->pnStripLength[i] + 2, GL_UNSIGNED_SHORT, (void*)(offset * sizeof(GLushort)));
			}
			else
			{
				// Non-Indexed Triangle strips
				glDrawArrays(GL_TRIANGLE_STRIP, offset, pMesh->pnStripLength[i] + 2);
			}
			offset += pMesh->pnStripLength[i] + 2;
		}
	}

	// Safely disable the vertex attribute arrays
	for (int i = 0; i < NumAttribute; ++i) { glDisableVertexAttribArray(i); }

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/******************************************************************************
@Discription           WaterPlane should be plane in xz dir, and up dir is y
@WaterPlanePosY_World  Y height In World space
*******************************************************************************/
void OGLES2PeaceWaterRender::RenderReflectionTex(Camera camera)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiReflectFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glEnable(GL_DEPTH_TEST);

	camera.ModifyProjectionForClipping(PVRTVec4(0.0, 1.0, 0.0, 3.0));

	//DrawBall(PVRTVec3(0, 0, 0), PVRTVec3(0.0, 0.0, 0.0));
	//DrawBall(camera, PVRTVec3(50, 100, 0), PVRTVec3(1.0, 0.0, 0.0));
	//DrawBall(camera, PVRTVec3(0, 150, 0), PVRTVec3(0.0, 1.0, 0.0));
	//DrawBall(camera, PVRTVec3(0, 100, 300), PVRTVec3(0.0, 0.0, 1.0));

	DrawSkybox(camera, 0);

	glDisable(GL_DEPTH_TEST);


	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);
}

void OGLES2PeaceWaterRender::RenderRefractionTex(Camera camera)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_auiRefractFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glEnable(GL_DEPTH_TEST);

	camera.ModifyProjectionForClipping(PVRTVec4(0.0, -1.0, 0.0, 3.0));

	//DrawBall(PVRTVec3(0, 0, 0), PVRTVec3(0.0, 0.0, 0.0));
	//DrawBall(camera, PVRTVec3(50, 100, 0), PVRTVec3(1.0, 0.0, 0.0));
	//DrawBall(camera, PVRTVec3(0, 150, 0), PVRTVec3(0.0, 1.0, 0.0));
	//DrawBall(camera, PVRTVec3(0, 100, 300), PVRTVec3(0.0, 0.0, 1.0));

	DrawSkybox(camera, 0);

	glDisable(GL_DEPTH_TEST);


	glBindFramebuffer(GL_FRAMEBUFFER, m_iOriginalFBO);
}

/*!****************************************************************************
@Function		DrawSkybox
@Description	Draws the skybox onto the screen.
******************************************************************************/
void OGLES2PeaceWaterRender::DrawSkybox(Camera & camera, int bDrawFog)
{
	glUseProgram(m_SkyboxProgram.uiId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiSkybox1_Tex);

	PVRTMat4 m_SkyboxLookAt = PVRTMat4::LookAtRH(PVRTVec3(0.0, 0.0, 0.0), PVRTVec3(0.0, 0.0, 1.0), PVRTVec3(0.0, 1.0, 0.0));
	PVRTVec3 euler = camera.getEulerAngle();
	PVRTMat4 m_ViewSkybox = PVRTMat4::RotationX(euler.x / 180.0f * PVRT_PI) * PVRTMat4::RotationY(euler.y / 180.0f * PVRT_PI) * PVRTMat4::RotationZ(euler.z / 180.0f * PVRT_PI) * m_SkyboxLookAt;

	PVRTMat4 CameraProjectionMatrix = camera.getProjectionMatrix();

	// Rotate and Translate the model matrix (if required)
	PVRTMat4 mModel(PVRTMat4::Identity());
	glUniformMatrix4fv(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eMMatrix], 1, GL_FALSE, mModel.ptr());

	// Set model view projection matrix
	PVRTMat4 mModelView(m_ViewSkybox * mModel);
	PVRTMat4 mMVP(CameraProjectionMatrix * mModelView);
	glUniformMatrix4fv(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	glUniform1i(m_SkyboxProgram.auiLoc[m_SkyboxProgram.ebDrawFog], bDrawFog);
	glUniform4fv(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eFogColor], 1, m_FogColor.ptr());
	glUniform1f(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eFogHeight], -100.0);
	glUniform1f(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eFogHeightRatio], m_FogHeightRatio * 2.0f);



	glDisable(GL_CULL_FACE);

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, m_puiSkyboxVbo);
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, NULL);

	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(VERTEX_ARRAY);

	for (int i = 0; i < 6; ++i)
	{
		// Draw primitive
		glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
	}

	// Safely disable the vertex attribute arrays
	glDisableVertexAttribArray(VERTEX_ARRAY);

	//glEnable(GL_CULL_FACE);
}

/*!****************************************************************************
@Function		DrawWaterPlanes
@Description	Draws the reflective and refractive ball onto the screen.
******************************************************************************/
void OGLES2PeaceWaterRender::DrawWaterPlanes(Camera & camera)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiSmallWaves_N_Tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiReflectRenderTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_uiRefractRenderTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiSkybox1_Tex);

	// Set model view projection matrix
	PVRTMat4 mModel, mModelView, mMVP;

	while (m_WaterRenderQueue.size()){
		// Use shader program
		glUseProgram(m_DefaultProgram.uiId);

		mModel = m_WaterRenderQueue.front()->GetModelMatrix();
		mModelView = camera.getViewMatrix() * mModel;
		mMVP = camera.getVPMatrix() * mModel;

		glUniformMatrix4fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eMVPMatrix], 1, GL_FALSE, mMVP.ptr());
		glUniformMatrix4fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eMMatrix], 1, GL_FALSE, mModel.ptr());

		PVRTMat4 mModel_IT = mModel;
		mModel_IT = mModel_IT.transpose();
		mModel_IT = mModel_IT.inverse();
		glUniformMatrix4fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eMMatrix_IT], 1, GL_FALSE, mModel_IT.ptr());

		m_ulTime += 1 * m_fDeltaTime;
		glUniform1f(m_DefaultProgram.auiLoc[m_DefaultProgram.eTime], m_ulTime);

		// Set eye position in model space
		PVRTVec3 vEyePosModel;
		vEyePosModel = mModelView.inverse() * PVRTVec4(camera.getPosition(), 1.0);
		glUniform3fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eEyePosModel], 1, vEyePosModel.ptr());

		// Calculate and set the model space light direction
		PVRTVec3 vLightDir = mModel.inverse() * m_globalLightDir;
		vLightDir = vLightDir.normalize();
		glUniform3fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eLightDirModel], 1, vLightDir.ptr());


		glUniform4fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eFogColor], 1, m_FogColor.ptr());
		glUniform1f(m_DefaultProgram.auiLoc[m_DefaultProgram.eFogDepthRatio], m_FogHeightRatio / 5.0f);

		// Now that the uniforms are set, call another function to actually draw the mesh
		int attirbutes[] = { VERTEX_ARRAY, NORMAL_ARRAY, TANGENT_ARRAY, BINORMAL_ARRAY, TEXCOORD_ARRAY };
		DrawMesh(0, m_WaterPlane.ModelPOD, &m_WaterPlane.VBO, &m_WaterPlane.IndexVBO, attirbutes);
		m_WaterRenderQueue.pop();
	}
}

/*!****************************************************************************
@Function		DrawBall
@Description	Draws the reflective and refractive ball onto the screen.
******************************************************************************/
void OGLES2PeaceWaterRender::DrawBall(Camera & camera, PVRTVec3 position, PVRTVec3 diffuseColor)
{
	// Set model view projection matrix
	PVRTMat4 mModel, mModelView, mMVP;

	m_Ball.SetPosition(position);

	mModel = m_Ball.GetModelMatrix();
	mModelView = camera.getViewMatrix() * mModel;
	mMVP = camera.getProjectionMatrix() * mModelView;

	if (diffuseColor == PVRTVec3(0.0, 0.0, 1.0)){
		if (m_Ball.NeedClip(mMVP)){
			m_Print3D.Print3D(0.0, 10.0, 1.0, PVRTRGBA(255, 255, 255, 255), "ballcliped");
		}
		else
		{
			m_Print3D.Print3D(0.0, 10.0, 1.0, PVRTRGBA(255, 255, 255, 255), "ballnotCliped");
		}
	}

	// Use shader program
	glUseProgram(m_BlinnPhongProgram.uiId);

	// Bind textures


	glUniformMatrix4fv(m_BlinnPhongProgram.auiLoc[m_BlinnPhongProgram.eMVPMatrix], 1, GL_FALSE, mMVP.ptr());


	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);
	glUniform3fv(m_BlinnPhongProgram.auiLoc[m_BlinnPhongProgram.eEyePosModel], 1, &vEyePosModel.x);

	// Calculate and set the model space light direction
	PVRTVec3 vLightDir = mModel.inverse() * m_globalLightDir;
	vLightDir = vLightDir.normalize();
	glUniform3fv(m_BlinnPhongProgram.auiLoc[m_BlinnPhongProgram.eLightDirModel], 1, vLightDir.ptr());

	glUniform3fv(m_BlinnPhongProgram.auiLoc[m_BlinnPhongProgram.eDiffuseColor], 1, diffuseColor.ptr());

	// Now that the uniforms are set, call another function to actually draw the mesh
	int attirbutes[] = { VERTEX_ARRAY, NORMAL_ARRAY };
	DrawMesh(0, m_Ball.ModelPOD, &m_Ball.VBO, &m_Ball.IndexVBO, attirbutes);
}

/*!****************************************************************************
@Function		DrawCube
@Description	Draws a simple Cube into the screen.
******************************************************************************/
void OGLES2PeaceWaterRender::DrawCube(Camera & camera){
	// Set model view projection matrix
	PVRTMat4 mModel, mModelView, mMVP;

	mModel = m_Cube.GetModelMatrix();
	mModelView = camera.getViewMatrix() * mModel;
	mMVP = camera.getProjectionMatrix() * mModelView;

	// Use shader program
	glUseProgram(m_DefaultProgram.uiId);

	glUniformMatrix4fv(m_DefaultProgram.auiLoc[m_BlinnPhongProgram.eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);
	glUniform3fv(m_DefaultProgram.auiLoc[m_BlinnPhongProgram.eEyePosModel], 1, &vEyePosModel.x);

	// Calculate and set the model space light direction
	PVRTVec3 vLightDir = mModel.inverse() * PVRTVec4(19, 22, -50, 0);
	vLightDir = vLightDir.normalize();
	glUniform3fv(m_DefaultProgram.auiLoc[m_BlinnPhongProgram.eLightDirModel], 1, vLightDir.ptr());

	// Calculate and set the model space eye position
	PVRTVec3 vEyePos = mModelView.inverse() * PVRTVec4(0.0f, 0.0f, 0.0f, 1.0f);
	glUniform3fv(m_DefaultProgram.auiLoc[m_BlinnPhongProgram.eEyePosModel], 1, vEyePos.ptr());

	// Now that the uniforms are set, call another function to actually draw the mesh

	int attirbutes[] = { 1, 2 };
	DrawMesh(0, m_Cube.ModelPOD, &m_Cube.VBO, &m_Cube.IndexVBO, attirbutes);
}

/*!****************************************************************************
@Function		NewDemo
@Return		PVRShell*		The demo supplied by the user
@Description	This function must be implemented by the user of the shell.
The user should return its PVRShell object defining the
behaviour of the application.
******************************************************************************/
PVRShell* NewDemo()
{
	return new OGLES2PeaceWaterRender;
}

/******************************************************************************
End of file (OGLES2PeaceWaterRender.cpp)
******************************************************************************/
