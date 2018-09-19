/******************************************************************************

@File         OGLES2OceanRender.cpp

@Title        Glass

@Version

@Copyright    Copyright (c) Imagination Technologies Limited.

@Platform     Independent

@Description  Demonstrates dynamic reflection and refraction by rendering two
halves of the scene to a single rectangular texture.

******************************************************************************/

#include "PVRShell.h"
#include "OGLES2Tools.h"
#include <iostream>
#include <limits.h>


/******************************************************************************
Constants
******************************************************************************/

const GLsizei g_ParaboloidTexSize = 512;

// Camera constants used to generate the projection matrix
const float g_fCamNear = 1.0f;
const float g_fCamFar = 8000.0;
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
	enum eUniformSampler{ eLargeWaves_HightTex, eLargeWaves_NormalTex, eSmallWaves_NormalTex, eSeaFoam1_Tex, eSkybox_Tex, eNumUniformSamplers };
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
	enum EUniform{ eMMatrix, eMVPMatrix, eFogColor, eFogHeightRatio, eNumUniforms };
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
const char c_szLargeWaves_H_TexFile[] = "LargeWaves_H_Tex.pvr";
const char c_szLargeWaves_N_TexFile[] = "LargeWaves_N_Tex.pvr";
const char c_szSmallWaves_N_TexFile[] = "SmallWaves_N_Tex.pvr";
const char c_szSeaFoam1_TexFile[] = "SeaFoam1_Tex.pvr";
const char c_szSkybox1_TexFile[] = "Skybox1_Tex.pvr";
const char c_szSkybox2_TexFile[] = "Skybox2_Tex.pvr";

// POD scene files
const char c_szBallFile[] = "Ball.pod";
const char c_szCube_testFile[] = "cube_test.pod";
const char c_szWaterPlaneFile[] = "WaterPlane.pod";


/*!****************************************************************************
Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2OceanRender : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	// 3D Models
	CPVRTModelPOD m_Ball;
	CPVRTModelPOD m_Cube;
	CPVRTModelPOD m_WaterPlane;

	// Projection, view and model matrices
	PVRTMat4 m_mProjection, m_mView, m_mViewLookAt;
	PVRTMat4 m_mViewRotation;
	float m_RotateAngelX, m_RotateAngelY;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiDefaultVertShader;
	GLuint m_uiDefaultFragShader;
	GLuint m_uiBPVertShader;
	GLuint m_uiBPFragShader;
	GLuint m_uiSkyboxVertShader;
	GLuint m_uiSkyboxFragShader;

	GLuint m_uiLargeWaves_H_Tex;
	GLuint m_uiLargeWaves_N_Tex;
	GLuint m_uiSmallWaves_N_Tex;
	GLuint m_uiSeaFoam1_Tex;
	GLuint m_uiSkybox1_Tex;
	GLuint m_uiSkybox2_Tex;

	GLuint*	m_puiBallVbo;
	GLuint*	m_puiBallIndexVbo;
	GLuint* m_puiCubeVbo;
	GLuint* m_puiCubeIndexVbo;
	GLuint* m_puiWaterPlaneVbo;
	GLuint* m_puiWaterPlaneIndexVbo;
	GLuint m_puiSkyboxVbo;

	// Skybox
	GLfloat* m_SkyboxVertices;
	GLfloat* m_SkyboxTexCoords;
	PVRTVec4 m_FogColor;
	float m_FogHeightRatio;

	//WorldSpace
	PVRTVec4 m_globalLightDir;
	PVRTVec4 m_globalViewPos;
	PVRTVec4 m_globalViewDir;
	PVRTVec4 m_globalViewUp;

	// Group shader programs and their uniform locations together
	DefaultProgram m_DefaultProgram;
	BlinnPhongProgram m_BlinnPhongProgram;
	SkyboxProgram m_SkyboxProgram;

	// Current time in milliseconds
	float m_ulTime;
	unsigned long m_ulPreviousTime, m_ulCurrentTime;
	float m_fElapsedTimeInSecs, m_fDeltaTime, m_fFrame, m_fCount;
	unsigned int m_uiFPS, m_uiFrameCount;

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

	template<class T>
	void DrawMesh(int i32NodeIndex, CPVRTModelPOD* pod, GLuint** ppuiVbos, GLuint** ppuiIbos, T & i32Attributes);

	void DrawBall(PVRTVec3 position, PVRTVec3 diffuseColor);
	void DrawCube();
	void DrawWaterPlane();
	void DrawSkybox();
};

/*!****************************************************************************
@Function		LoadModels
@Output		pErrorStr		A string describing the error on failure
@Return		bool			true if no error occured
@Description	Loads the Models required for this training course
******************************************************************************/
bool OGLES2OceanRender::LoadModels(CPVRTString* const pErrorStr){
	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the ball
	if (m_Ball.ReadFromFile(c_szBallFile) != PVR_SUCCESS)
	{
		*pErrorStr = "ERROR: Couldn't load the .pod file\n";
		return false;
	}

	//Load the cube
	if (m_Cube.ReadFromFile(c_szCube_testFile) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Couldn't load the Cube_test .pod file\n";
		return false;
	}

	//Load the WaterPlane
	if (m_WaterPlane.ReadFromFile(c_szWaterPlaneFile) != PVR_SUCCESS){
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
bool OGLES2OceanRender::LoadTextures(CPVRTString* const pErrorStr)
{
	/*if (PVRTTextureLoadFromPVR(c_szHairDiffWHiteTexFile, &m_uiHairDiffWhiteTex) != PVR_SUCCESS){
	*pErrorStr = "ERROR: Failed to load Hair Diffuse Tex.";
	return false;
	}*/
	if (PVRTTextureLoadFromPVR(c_szLargeWaves_H_TexFile, &m_uiLargeWaves_H_Tex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load LargeWaves_Hight Tex.";
		return false;
	}
	if (PVRTTextureLoadFromPVR(c_szLargeWaves_N_TexFile, &m_uiLargeWaves_N_Tex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load LargeWaves_Normal Tex.";
		return false;
	}
	if (PVRTTextureLoadFromPVR(c_szSmallWaves_N_TexFile, &m_uiSmallWaves_N_Tex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load SmallWaves_Normal Tex.";
		return false;
	}
	if (PVRTTextureLoadFromPVR(c_szSeaFoam1_TexFile, &m_uiSeaFoam1_Tex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load SeaFoam1 Tex";
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

	return true;
}

bool OGLES2OceanRender::LoadSkyboxShader(CPVRTString* pErrorStr)
{
	const char* g_aszUniformNames[] =
	{
		"ModelMatrix", "MVPMatrix", "FogColor", "FogHeightRatio"
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

bool OGLES2OceanRender::LoadBlinnPhongShader(CPVRTString* pErrorStr)
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

bool OGLES2OceanRender::LoadDefaultShader(CPVRTString* pErrorStr)
{
	const char* g_aszUniformNames[] =
	{
		"MVPMatrix", "MMatrix", "MMatrix_IT", "LightDirModel", "EyePosModel", "_Time", "FogColor", "FogDepthRatio"
	};
	const char* g_aszUniformSamplerNames[] =
	{
		"LargeWaves_HightTex", "LargeWaves_NormalTex", "SmallWaves_NormalTex", "SeaFoam1_Tex", "Skybox_Tex"
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
bool OGLES2OceanRender::LoadShaders(CPVRTString* pErrorStr)
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
void OGLES2OceanRender::LoadVbos()
{
	m_puiBallVbo = 0;
	m_puiBallIndexVbo = 0;
	m_puiCubeVbo = 0;
	m_puiCubeIndexVbo = 0;
	m_puiWaterPlaneVbo = 0;
	m_puiWaterPlaneIndexVbo = 0;

	if (!m_puiBallVbo)      { m_puiBallVbo = new GLuint[m_Ball.nNumMesh]; }
	if (!m_puiBallIndexVbo) { m_puiBallIndexVbo = new GLuint[m_Ball.nNumMesh]; }
	glGenBuffers(m_Ball.nNumMesh, m_puiBallVbo);
	for (unsigned int i = 0; i < m_Ball.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Ball.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_puiBallVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiBallIndexVbo[i] = 0;
		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_puiBallIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiBallIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	if (!m_puiCubeVbo)      { m_puiCubeVbo = new GLuint[m_Cube.nNumMesh]; }
	if (!m_puiCubeIndexVbo) { m_puiCubeIndexVbo = new GLuint[m_Cube.nNumMesh]; }
	glGenBuffers(m_Cube.nNumMesh, m_puiCubeVbo);
	for (unsigned int i = 0; i < m_Cube.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_Cube.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_puiCubeVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiCubeIndexVbo[i] = 0;
		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_puiCubeIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiCubeIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	if (!m_puiWaterPlaneVbo)      { m_puiWaterPlaneVbo = new GLuint[m_WaterPlane.nNumMesh]; }
	if (!m_puiWaterPlaneIndexVbo) { m_puiWaterPlaneIndexVbo = new GLuint[m_WaterPlane.nNumMesh]; }
	glGenBuffers(m_WaterPlane.nNumMesh, m_puiWaterPlaneVbo);
	for (unsigned int i = 0; i < m_WaterPlane.nNumMesh; ++i)
	{
		// Load vertex data into buffer object
		SPODMesh& Mesh = m_WaterPlane.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_puiWaterPlaneVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		// Load index data into buffer object if available
		m_puiWaterPlaneIndexVbo[i] = 0;
		if (Mesh.sFaces.pData)
		{
			glGenBuffers(1, &m_puiWaterPlaneIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiWaterPlaneIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}


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
bool OGLES2OceanRender::InitApplication()
{
	CPVRTString pErrorStr;
	if (!LoadModels(&pErrorStr)){
		PVRShellSet(prefExitMessage, pErrorStr.c_str());
		return false;
	}
	PVRShellSet(prefSwapInterval, 0);

	m_ulTime = 0.0;

	m_ulCurrentTime = PVRShellGetTime();
	m_ulPreviousTime = m_ulCurrentTime;
	m_fCount = 0;
	m_uiFrameCount = 0;
	m_uiFPS = 0;

	m_RotateAngelX = 0;
	m_RotateAngelY = 0;

	m_FogColor = PVRTVec4(0.431f, 0.373f, 0.333f, 1.0f);
	m_FogHeightRatio = 1.0f / 1000.0f;

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
bool OGLES2OceanRender::QuitApplication()
{
	// Free the memory allocated for the scene
	/*m_Ball.Destroy();
	m_Balloon.Destroy();*/

	delete[] m_puiBallVbo;
	delete[] m_puiBallIndexVbo;
	delete[] m_puiCubeVbo;
	delete[] m_puiCubeIndexVbo;
	delete[] m_puiWaterPlaneVbo;
	delete[] m_puiWaterPlaneIndexVbo;
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
bool OGLES2OceanRender::InitView()
{
	CPVRTString ErrorStr;

	// Create the skybox
	PVRTCreateSkybox(4000, true, 512, &m_SkyboxVertices, &m_SkyboxTexCoords);
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
	Calculate the projection and view matrices
	*/
	m_globalLightDir = PVRTVec4(0, 0, 0, 0) - PVRTVec4(0, -1, -1, 0);
	m_globalViewPos = PVRTVec4(0, 200, 0, 0);
	m_globalViewDir = PVRTVec4(0, 0, 11, 0);
	m_globalViewUp = PVRTVec4(0, 1, 0, 0);

	m_mViewLookAt = PVRTMat4::LookAtRH(m_globalViewPos, m_globalViewPos + m_globalViewDir, m_globalViewUp);
	m_mView = m_mViewLookAt;
	m_mProjection = PVRTMat4::PerspectiveFovRH(g_fCamFOV, (float)PVRShellGet(prefWidth) / (float)PVRShellGet(prefHeight), g_fCamNear,
		g_fCamFar, PVRTMat4::OGL, bRotate);

	/*
	Set OpenGL ES render states needed for this training course
	*/

	// Use a nice bright blue as clear colour
	glClearColor(0.6f, 0.8f, 1.0f, 0.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	return true;
}

/*!****************************************************************************
@Function		ReleaseView
@Return		bool		true if no error occured
@Description	Code in ReleaseView() will be called by PVRShell when the
application quits or before a change in the rendering context.
******************************************************************************/
bool OGLES2OceanRender::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiLargeWaves_H_Tex);
	glDeleteTextures(1, &m_uiLargeWaves_N_Tex);
	glDeleteTextures(1, &m_uiSmallWaves_N_Tex);
	glDeleteTextures(1, &m_uiSeaFoam1_Tex);

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
	glDeleteBuffers(m_Ball.nNumMesh, m_puiBallVbo);
	glDeleteBuffers(m_Ball.nNumMesh, m_puiBallIndexVbo);
	glDeleteBuffers(m_WaterPlane.nNumMesh, m_puiWaterPlaneVbo);
	glDeleteBuffers(m_WaterPlane.nNumMesh, m_puiWaterPlaneIndexVbo);

	// Delete renderbuffers


	// Delete framebuffers

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
bool OGLES2OceanRender::RenderScene()
{
	ShowFPS();
	if (PVRShellIsKeyPressed(PVRShellKeyNameUP)){
		m_RotateAngelX += 10.0f / 180.0f *PVRT_PI;
		//m_mView *= PVRTMat4::RotationZ(10.0f / 180.0f * PVRT_PI);
		//m_mView *= PVRTMat4::Translation(0.0, 0.0, -1.0);
		//m_mModel *= PVRTMat4::RotationX(10.0f / 180.0f * PVRT_PI);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN)){
		m_RotateAngelX += -10.0f / 180.0f *PVRT_PI;
		//m_mView *= PVRTMat4::RotationZ(-10.0f / 180.0f * PVRT_PI);
		//m_mView *= PVRTMat4::Translation(0.0, 0.0, 1.0);
		//m_mModel *= PVRTMat4::RotationX(-10.0f / 180.0f * PVRT_PI);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT)){
		m_RotateAngelY += 10.0f / 180.0f * PVRT_PI;
		//m_mView *= PVRTMat4::RotationY(10.0f / 180.0f * PVRT_PI);
		//m_mView *= PVRTMat4::RotationY(-10.0 / 180.0 * PVRT_PI);
		//m_mModel *= PVRTMat4::RotationY(10.0f / 180.0f * PVRT_PI);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT)){
		m_RotateAngelY += -10.0f / 180.0f *PVRT_PI;
		//m_mView *= PVRTMat4::RotationY(-10.0f / 180.0f * PVRT_PI);
		//m_mView *= PVRTMat4::RotationY(10.0 / 180.0 * PVRT_PI);
		//m_mModel *= PVRTMat4::RotationY(-10.0f / 180.0f * PVRT_PI);
	}
	m_mViewRotation = PVRTMat4::RotationX(m_RotateAngelX) * PVRTMat4::RotationY(m_RotateAngelY);
	m_mView = m_mViewRotation * m_mViewLookAt;
	//DrawIntoParaboloids(PVRTVec3(0, 0, 0));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Clear the color


	glEnable(GL_DEPTH_TEST);
	//DrawBall(PVRTVec3(0, 0, 0), PVRTVec3(0.0, 0.0, 0.0));
	DrawBall(PVRTVec3(100, 0, 0), PVRTVec3(1.0, 0.0, 0.0));
	DrawBall(PVRTVec3(0, 100, 0), PVRTVec3(0.0, 1.0, 0.0));
	DrawBall(PVRTVec3(0, 0, 100), PVRTVec3(0.0, 0.0, 1.0));
	DrawWaterPlane();

	DrawSkybox();

	glDisable(GL_DEPTH_TEST);

	m_Print3D.Flush();
	return true;
}

/*!****************************************************************************
@Function		UpdateScene
@Description	Moves the scene.
******************************************************************************/
void OGLES2OceanRender::ShowFPS()
{
	m_uiFrameCount++;

	m_ulPreviousTime = m_ulCurrentTime;
	m_ulCurrentTime = PVRShellGetTime();

	m_fElapsedTimeInSecs = m_ulCurrentTime * 0.001f;
	m_fDeltaTime = ((float)(m_ulCurrentTime - m_ulPreviousTime))*0.001f;

	m_fCount += m_fDeltaTime;

	if (m_fCount >= 1.0f)			// Update FPS once a second
	{
		m_uiFPS = m_uiFrameCount;
		m_uiFrameCount = 0;
		m_fCount = 0;
	}

	m_Print3D.Print3D(0.0, 0.0, 1.0, PVRTRGBA(255, 255, 255, 255), "FPS: %2i", m_uiFPS);
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
void OGLES2OceanRender::DrawMesh(int i32NodeIndex, CPVRTModelPOD* pod, GLuint** ppuiVbos, GLuint** ppuiIbos, T & i32Attributes)
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

/*!****************************************************************************
@Function		DrawSkybox
@Description	Draws the skybox onto the screen.
******************************************************************************/
void OGLES2OceanRender::DrawSkybox()
{
	glUseProgram(m_SkyboxProgram.uiId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiSkybox1_Tex);

	PVRTMat4 m_ViewSkybox = m_mViewRotation * PVRTMat4::LookAtRH(PVRTVec3(0.0, 0.0, 0.0), m_globalViewDir, m_globalViewUp);
	PVRTMat4 mVP = m_mProjection * m_ViewSkybox;
	PVRTMat4 mInvVP = mVP.inverse();

	// Rotate and Translate the model matrix (if required)
	PVRTMat4 mModel(PVRTMat4::Identity());
	glUniformMatrix4fv(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eMMatrix], 1, GL_FALSE, mModel.ptr());

	// Set model view projection matrix
	PVRTMat4 mModelView(m_mView * mModel);
	PVRTMat4 mMVP(m_mProjection * mModelView);
	glUniformMatrix4fv(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	glUniform4fv(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eFogColor], 1, m_FogColor.ptr());
	glUniform1f(m_SkyboxProgram.auiLoc[m_SkyboxProgram.eFogHeightRatio], m_FogHeightRatio);



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
@Function		DrawWaterPlane
@Description	Draws the reflective and refractive ball onto the screen.
******************************************************************************/
void OGLES2OceanRender::DrawWaterPlane()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiLargeWaves_H_Tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiLargeWaves_N_Tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_uiSmallWaves_N_Tex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_uiSeaFoam1_Tex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiSkybox1_Tex);

	// Set model view projection matrix
	PVRTMat4 mModel, mModelView, mMVP;

	mModel = PVRTMat4::Identity();
	mModel *= PVRTMat4::Scale(50, 1, 50);

	mModelView = m_mView * mModel;
	mMVP = m_mProjection * mModelView;

	// Use shader program
	glUseProgram(m_DefaultProgram.uiId);

	glUniformMatrix4fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eMVPMatrix], 1, GL_FALSE, mMVP.ptr());
	glUniformMatrix4fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eMMatrix], 1, GL_FALSE, mModel.ptr());

	PVRTMat4 mModel_IT = mModel;
	mModel_IT = mModel_IT.transpose();
	mModel_IT = mModel_IT.inverse();
	glUniformMatrix4fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eMMatrix_IT], 1, GL_FALSE, mModel_IT.ptr());

	m_ulTime += 1 * m_fDeltaTime;
	glUniform1f(m_DefaultProgram.auiLoc[m_DefaultProgram.eTime], m_ulTime);

	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);
	glUniform3fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eEyePosModel], 1, &vEyePosModel.x);

	// Calculate and set the model space light direction
	PVRTVec3 vLightDir = mModel.inverse() * m_globalLightDir;
	vLightDir = vLightDir.normalize();
	glUniform3fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eLightDirModel], 1, vLightDir.ptr());


	glUniform4fv(m_DefaultProgram.auiLoc[m_DefaultProgram.eFogColor], 1, m_FogColor.ptr());
	glUniform1f(m_DefaultProgram.auiLoc[m_DefaultProgram.eFogDepthRatio], m_FogHeightRatio / 3.0f);

	// Now that the uniforms are set, call another function to actually draw the mesh
	int attirbutes[] = { VERTEX_ARRAY, NORMAL_ARRAY, TANGENT_ARRAY, BINORMAL_ARRAY, TEXCOORD_ARRAY };
	DrawMesh(0, &m_WaterPlane, &m_puiWaterPlaneVbo, &m_puiWaterPlaneIndexVbo, attirbutes);
}

/*!****************************************************************************
@Function		DrawBall
@Description	Draws the reflective and refractive ball onto the screen.
******************************************************************************/
void OGLES2OceanRender::DrawBall(PVRTVec3 position, PVRTVec3 diffuseColor)
{
	// Set model view projection matrix
	PVRTMat4 mModel, mModelView, mMVP;

	mModel = PVRTMat4::Identity();
	mModel *= PVRTMat4::Translation(position);
	mModel *= PVRTMat4::Scale(2, 2, 2);

	mModelView = m_mView * mModel;
	mMVP = m_mProjection * mModelView;

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
	DrawMesh(0, &m_Ball, &m_puiBallVbo, &m_puiBallIndexVbo, attirbutes);
}

/*!****************************************************************************
@Function		DrawCube
@Description	Draws a simple Cube into the screen.
******************************************************************************/
void OGLES2OceanRender::DrawCube(){
	// Set model view projection matrix
	PVRTMat4 mModel, mModelView, mMVP;

	mModel = PVRTMat4::Scale(5.0f, 5.0f, 1.0f);
	//mModel = mModel * PVRTMat4::Translation()
	mModelView = m_mView * mModel;
	mMVP = m_mProjection * mModelView;

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
	DrawMesh(0, &m_Cube, &m_puiCubeVbo, &m_puiCubeIndexVbo, attirbutes);
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
	return new OGLES2OceanRender;
}

/******************************************************************************
End of file (OGLES2OceanRender.cpp)
******************************************************************************/
