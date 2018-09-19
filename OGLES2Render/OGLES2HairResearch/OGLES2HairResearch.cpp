/******************************************************************************

@File         OGLES2Glass.cpp

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
const float g_fCamFar = 500.0f;
const float g_fCamFOV = PVRT_PI / 3.0f;

// Vertex attributes
enum EVertexAttrib
{
	VERTEX_ARRAY, NORMAL_ARRAY, TANGENT_ARRAY, BINORMAL_ARRAY, TEXCOORD_ARRAY1, eNumAttribs
};

const char* g_aszAttribNames[] =
{
	"inVertex", "inNormal", "inTangent", "inBiNormal", "inTexCoords"
};

// Shader uniforms
enum EUniform
{
	eMVPMatrix, eMVMatrix, eMMatrix, eMMatrix_I, eMMatrix_IT, eMVMatrix_IT, eInvVPMatrix, eLightDir, eEyePos, ePass, eDiffuseColor, eNumUniforms
};

const char* g_aszUniformNames[] =
{
	"MVPMatrix", "MVMatrix", "MMatrix", "MMatrix_I", "MMatrix_IT", "MVMatrix_IT", "InvVPMatrix", "LightDir", "EyePos", "Pass", "diffuseColor"
};

/******************************************************************************
Content file names
******************************************************************************/

// Source and binary shaders
const char c_szFragShaderSrcFile[] = "DefaultFragShader.fsh";
const char c_szFragShaderBinFile[] = "DefaultFragShader.fsc";
const char c_szVertShaderSrcFile[] = "DefaultVertShader.vsh";
const char c_szVertShaderBinFile[] = "DefaultVertShader.vsc";

const char c_szBPFragShaderSrcFile[] = "BlinnPhongFragShader.fsh";
const char c_szBPFragShaderBinFile[] = "BlinnPhongFragShader.fsc";
const char c_szBPVertShaderSrcFile[] = "BlinnPhongVertShader.vsh";
const char c_szBPVertShaderBinFile[] = "BlinnPhongVertShader.vsc";

// PVR texture files
const char c_szHairDiffWHiteTexFile[] = "Hair_diff_whiteTex.pvr";
const char c_szHairFlowT2TexFile[] = "Hair_flow_t2Tex.pvr";
const char c_szHairNMTexFile[] = "Hair_nmTex.pvr";
const char c_szHeadDiffTexFile[] = "Head_diffTex.pvr";

// POD scene files
const char c_szBallFile[] = "Ball.pod";
const char c_szCube_testFile[] = "cube_test.pod";
const char c_szHairModelFile[] = "HairModel.pod";
const char c_szHeadModelFile[] = "HeadModel.pod";

/*!****************************************************************************
Class implementing the PVRShell functions.
******************************************************************************/
class OGLES2Glass : public PVRShell
{
	// Print3D class used to display text
	CPVRTPrint3D m_Print3D;

	// 3D Models
	CPVRTModelPOD m_Ball;
	CPVRTModelPOD m_Cube_test;
	CPVRTModelPOD m_HairModel;
	CPVRTModelPOD m_HeadModel;

	// Projection, view and model matrices
	PVRTMat4 m_mProjection, m_mView;

	// OpenGL handles for shaders, textures and VBOs
	GLuint m_uiDefaultVertShader;
	GLuint m_uiDefaultFragShader;
	GLuint m_uiBPVertShader;
	GLuint m_uiBPFragShader;

	GLuint m_uiHairDiffWhiteTex;
	GLuint m_uiHairNMTex;
	GLuint m_uiHairFlowT2Tex;
	GLuint m_uiHeadDiffTex;

	GLuint*	m_puiBallVbo;
	GLuint*	m_puiBallIndexVbo;
	GLuint* m_puiCube_testVbo;
	GLuint* m_puiCube_testIndexVbo;
	GLuint* m_puiHairModelVbo;
	GLuint* m_puiHairModelIndexVbo;
	GLuint* m_puiHeadVbo;
	GLuint* m_puiHeadIndexVbo;
	GLuint	m_uiSquareVbo;

	//WorldSpace
	PVRTVec4 m_globalLightDir = PVRTVec4(0, 0, 0, 0) - PVRTVec4(0, -1, 0, 0);
	PVRTVec4 m_globalViewPos = PVRTVec4(0, 0, -20, 0);

	// Group shader programs and their uniform locations together
	struct Program
	{
		GLuint uiId;
		GLuint auiLoc[eNumUniforms];
	}
	m_DefaultProgram, m_BlinnPhongProgram;

	// Current time in milliseconds
	unsigned long m_ulTime;

	// Rotation angle for the model
	float m_afAngles[2];

	int m_iEffect;

public:
	virtual bool InitApplication();
	virtual bool InitView();
	virtual bool ReleaseView();
	virtual bool QuitApplication();
	virtual bool RenderScene();

private:
	bool LoadTextures(CPVRTString* pErrorStr);
	bool LoadShaders(CPVRTString* pErrorStr);
	void LoadVbos();

	void UpdateScene();

	void DrawMesh(int i32NodeIndex, CPVRTModelPOD* pod, GLuint** ppuiVbos, GLuint** ppuiIbos, int i32NumAttributes);

	void DrawBall(PVRTVec3 position, PVRTVec3 diffuseColor);
	void DrawCube();
	void DrawHair(int pass);
	void DrawHead();
};

/*!****************************************************************************
@Function		LoadTextures
@Output		pErrorStr		A string describing the error on failure
@Return		bool			true if no error occured
@Description	Loads the textures required for this training course
******************************************************************************/
bool OGLES2Glass::LoadTextures(CPVRTString* const pErrorStr)
{
	if (PVRTTextureLoadFromPVR(c_szHairDiffWHiteTexFile, &m_uiHairDiffWhiteTex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load Hair Diffuse Tex.";
		return false;
	}

	if (PVRTTextureLoadFromPVR(c_szHairNMTexFile, &m_uiHairNMTex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load Hair Normal Tex.";
		return false;
	}

	if (PVRTTextureLoadFromPVR(c_szHairFlowT2TexFile, &m_uiHairFlowT2Tex) != PVR_SUCCESS){
		*pErrorStr = "ERROR: Failed to load Hair Flow Tex.";
		return false;
	}

	if (PVRTTextureLoadFromPVR(c_szHeadDiffTexFile, &m_uiHeadDiffTex) != PVR_SUCCESS){
		*pErrorStr = "ERRRO: Failed to load HeadDiffuse Tex.";
		return false;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

/*!****************************************************************************
@Function		LoadShaders
@Output		pErrorStr		A string describing the error on failure
@Return		bool			true if no error occured
@Description	Loads and compiles the shaders and links the shader programs
required for this training course
******************************************************************************/
bool OGLES2Glass::LoadShaders(CPVRTString* pErrorStr)
{
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
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_DefaultProgram.auiLoc[i] = glGetUniformLocation(m_DefaultProgram.uiId, g_aszUniformNames[i]);
	}

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
	for (int i = 0; i < eNumUniforms; ++i)
	{
		m_BlinnPhongProgram.auiLoc[i] = glGetUniformLocation(m_BlinnPhongProgram.uiId, g_aszUniformNames[i]);
	}



	return true;
}

/*!****************************************************************************
@Function		LoadVbos
@Description	Loads the mesh data required for this training course into
vertex buffer objects
******************************************************************************/
void OGLES2Glass::LoadVbos()
{
	//if (!m_puiCube_testVbo)      { m_puiCube_testVbo = new GLuint[m_Cube_test.nNumMesh]; }
	//if (!m_puiCube_testIndexVbo) { m_puiCube_testIndexVbo = new GLuint[m_Cube_test.nNumMesh]; }

	///*
	//Load vertex data of all meshes in the scene into VBOs

	//The meshes have been exported with the "Interleave Vectors" option,
	//so all data is interleaved in the buffer at pMesh->pInterleaved.
	//Interleaving data improves the memory access pattern and cache efficiency,
	//thus it can be read faster by the hardware.
	//*/
	//glGenBuffers(m_Cube_test.nNumMesh, m_puiCube_testVbo);
	//for (unsigned int i = 0; i < m_Cube_test.nNumMesh; ++i)
	//{
	//	// Load vertex data into buffer object
	//	SPODMesh& Mesh = m_Cube_test.pMesh[i];
	//	unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
	//	glBindBuffer(GL_ARRAY_BUFFER, m_puiCube_testVbo[i]);
	//	glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

	//	// Load index data into buffer object if available
	//	m_puiCube_testIndexVbo[i] = 0;
	//	if (Mesh.sFaces.pData)
	//	{
	//		glGenBuffers(1, &m_puiCube_testIndexVbo[i]);
	//		uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
	//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiCube_testIndexVbo[i]);
	//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
	//	}
	//}

	if (!m_puiBallVbo)      { m_puiBallVbo = new GLuint[m_Ball.nNumMesh]; }
	if (!m_puiBallIndexVbo) { m_puiBallIndexVbo = new GLuint[m_Ball.nNumMesh]; }

	/*
	Load vertex data of all meshes in the scene into VBOs

	The meshes have been exported with the "Interleave Vectors" option,
	so all data is interleaved in the buffer at pMesh->pInterleaved.
	Interleaving data improves the memory access pattern and cache efficiency,
	thus it can be read faster by the hardware.
	*/
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

	//Load HairModel Vbo
	if (!m_puiHairModelVbo)      { m_puiHairModelVbo = new GLuint[m_HairModel.nNumMesh]; }
	if (!m_puiHairModelIndexVbo) { m_puiHairModelIndexVbo = new GLuint[m_HairModel.nNumMesh]; }

	glGenBuffers(m_HairModel.nNumMesh, m_puiHairModelVbo);
	for (unsigned int i = 0; i < m_HairModel.nNumMesh; i++){
		SPODMesh& Mesh = m_HairModel.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_puiHairModelVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		m_puiHairModelIndexVbo[i] = 0;
		if (Mesh.sFaces.pData){
			glGenBuffers(1, &m_puiHairModelIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiHairModelIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	//Load Plane Vbo
	if (!m_puiHeadVbo)      { m_puiHeadVbo = new GLuint[m_HeadModel.nNumMesh]; }
	if (!m_puiHeadIndexVbo) { m_puiHeadIndexVbo = new GLuint[m_HeadModel.nNumMesh]; }

	glGenBuffers(m_HeadModel.nNumMesh, m_puiHeadVbo);
	for (unsigned int i = 0; i < m_HeadModel.nNumMesh; i++){
		SPODMesh& Mesh = m_HeadModel.pMesh[i];
		unsigned int uiSize = Mesh.nNumVertex * Mesh.sVertex.nStride;
		glBindBuffer(GL_ARRAY_BUFFER, m_puiHeadVbo[i]);
		glBufferData(GL_ARRAY_BUFFER, uiSize, Mesh.pInterleaved, GL_STATIC_DRAW);

		m_puiHeadIndexVbo[i] = 0;
		if (Mesh.sFaces.pData){
			glGenBuffers(1, &m_puiHeadIndexVbo[i]);
			uiSize = PVRTModelPODCountIndices(Mesh) * sizeof(GLshort);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_puiHeadIndexVbo[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, uiSize, Mesh.sFaces.pData, GL_STATIC_DRAW);
		}
	}

	static GLfloat fQuadVertices[] =
	{
		-1, 1, 0.9999f,
		-1, -1, 0.9999f,
		1, 1, 0.9999f,
		1, 1, 0.9999f,
		-1, -1, 0.9999f,
		1, -1, 0.9999f
	};

	glGenBuffers(1, &m_uiSquareVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiSquareVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, fQuadVertices, GL_STATIC_DRAW);

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
bool OGLES2Glass::InitApplication()
{
	m_puiBallVbo = 0;
	m_puiBallIndexVbo = 0;
	m_puiCube_testVbo = 0;
	m_puiCube_testIndexVbo = 0;
	m_puiHairModelVbo = 0;
	m_puiHairModelIndexVbo = 0;
	m_puiHeadVbo = 0;
	m_puiHeadIndexVbo = 0;


	// Get and set the read path for content files
	CPVRTResourceFile::SetReadPath((char*)PVRShellGet(prefReadPath));

	// Get and set the load/release functions for loading external files.
	// In the majority of cases the PVRShell will return NULL function pointers implying that
	// nothing special is required to load external files.
	CPVRTResourceFile::SetLoadReleaseFunctions(PVRShellGet(prefLoadFileFunc), PVRShellGet(prefReleaseFileFunc));

	// Load the mask
	if (m_Ball.ReadFromFile(c_szBallFile) != PVR_SUCCESS)
	{
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
		return false;
	}

	//// Load the balloon
	//if (m_Balloon.ReadFromFile(c_szBalloonFile) != PVR_SUCCESS)
	//{
	//	PVRShellSet(prefExitMessage, "ERROR: Couldn't load the .pod file\n");
	//	return false;
	//}

	

	if (m_HairModel.ReadFromFile(c_szHairModelFile) != PVR_SUCCESS){
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the HairMode .pod file\n");
	}

	if (m_HeadModel.ReadFromFile(c_szHeadModelFile) != PVR_SUCCESS){
		PVRShellSet(prefExitMessage, "ERROR: Couldn't load the Plane .pod file\n");
	}

	m_ulTime = ULONG_MAX;

	m_afAngles[0] = 0.0f;
	m_afAngles[1] = 0.0f;

	m_iEffect = 0;

	//初始化观察矩阵
	m_mView = PVRTMat4::LookAtRH(m_globalViewPos, PVRTVec3(0, 0, 0), PVRTVec3(0, 1, 0));

	PVRShellSet(prefAASamples, 8);

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
bool OGLES2Glass::QuitApplication()
{
	// Free the memory allocated for the scene
	/*m_Ball.Destroy();
	m_Balloon.Destroy();*/

	delete[] m_puiBallVbo;
	delete[] m_puiBallIndexVbo;
	delete[] m_puiCube_testVbo;
	delete[] m_puiCube_testIndexVbo;
	delete[] m_puiHairModelVbo;
	delete[] m_puiHairModelIndexVbo;
	delete[] m_puiHeadVbo;
	delete[] m_puiHeadIndexVbo;

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
bool OGLES2Glass::InitView()
{
	CPVRTString ErrorStr;

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

	// Set the sampler2D uniforms to corresponding texture units
	glUseProgram(m_DefaultProgram.uiId);
	glUniform1i(glGetUniformLocation(m_DefaultProgram.uiId, "diffuseMap"), 0);
	glUniform1i(glGetUniformLocation(m_DefaultProgram.uiId, "normalMap"), 1);
	glUniform1i(glGetUniformLocation(m_DefaultProgram.uiId, "specFlowMap"), 2);


	/*
	Calculate the projection and view matrices
	*/
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
bool OGLES2Glass::ReleaseView()
{
	// Delete textures
	glDeleteTextures(1, &m_uiHairDiffWhiteTex);
	glDeleteTextures(1, &m_uiHairFlowT2Tex);
	glDeleteTextures(1, &m_uiHairNMTex);

	// Delete program objects
	glDeleteProgram(m_DefaultProgram.uiId);
	glDeleteProgram(m_BlinnPhongProgram.uiId);

	// Delete shader objects
	glDeleteShader(m_uiDefaultVertShader);
	glDeleteShader(m_uiDefaultFragShader);
	glDeleteShader(m_uiBPVertShader);
	glDeleteShader(m_uiBPFragShader);

	// Delete buffer objects
	glDeleteBuffers(m_Ball.nNumMesh, m_puiBallVbo);
	glDeleteBuffers(m_Ball.nNumMesh, m_puiBallIndexVbo);
	//glDeleteBuffers(m_Balloon.nNumMesh, m_puiBalloonVbo);
	//glDeleteBuffers(m_Balloon.nNumMesh, m_puiBalloonIndexVbo);
	glDeleteBuffers(m_HairModel.nNumMesh, m_puiHairModelVbo);
	glDeleteBuffers(m_HairModel.nNumMesh, m_puiHairModelIndexVbo);
	glDeleteBuffers(m_HeadModel.nNumMesh, m_puiHeadVbo);
	glDeleteBuffers(m_HeadModel.nNumMesh, m_puiHeadIndexVbo);

	glDeleteBuffers(1, &m_uiSquareVbo);

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
bool OGLES2Glass::RenderScene()
{
	if (PVRShellIsKeyPressed(PVRShellKeyNameUP)){
		m_mView *= PVRTMat4::RotationX(10.0f / 180.0f * PVRT_PI);
		//m_mView *= PVRTMat4::Translation(0.0, 0.0, -1.0);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameDOWN)){
		m_mView *= PVRTMat4::RotationX(-10.0f / 180.0f * PVRT_PI);
		//m_mView *= PVRTMat4::Translation(0.0, 0.0, 1.0);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameLEFT)){
		m_mView *= PVRTMat4::RotationY(10.0f / 180.0f * PVRT_PI);
		//m_mView *= PVRTMat4::RotationY(-10.0 / 180.0 * PVRT_PI);
	}
	if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT)){
		m_mView *= PVRTMat4::RotationY(-10.0f / 180.0f * PVRT_PI);
		//m_mView *= PVRTMat4::RotationY(10.0 / 180.0 * PVRT_PI);
	}
	//if (PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))	{ m_iEffect += 1; }
	//m_iEffect = (m_iEffect + g_iNumEffects) % g_iNumEffects;


	UpdateScene();

	//DrawIntoParaboloids(PVRTVec3(0, 0, 0));

	// Clear the color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	DrawHead();
	glDisable(GL_DEPTH_TEST);

	//Draw Hair
	//1
	glEnable(GL_DEPTH_TEST); //Z test
	glDepthFunc(GL_LEQUAL);

	DrawHair(1); //绘制完全不透明

	glDisable(GL_DEPTH_TEST);

	//////2
	glEnable(GL_DEPTH_TEST); //Z test
	glEnable(GL_BLEND);//混合，绘制半透明
	glEnable(GL_CULL_FACE);

	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CW);
	glCullFace(GL_FRONT);//剔除正面

	DrawHair(2);

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	/////3
	glEnable(GL_DEPTH_TEST); //Z test
	glEnable(GL_BLEND);//混合，绘制半透明
	glEnable(GL_CULL_FACE);

	glDepthFunc(GL_LESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);//剔除背面

	DrawHair(3);

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//////
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	// Draw the ball
	DrawBall(PVRTVec3(0, 0, 0), PVRTVec3(0, 0, 0));
	DrawBall(PVRTVec3(10, 0, 0), PVRTVec3(1, 0, 0));
	DrawBall(PVRTVec3(0, 10, 0), PVRTVec3(0, 1, 0));
	DrawBall(PVRTVec3(0, 0, 10), PVRTVec3(0, 0, 1));

	glDisable(GL_DEPTH_TEST);
	
	
	// Displays the demo name using the tools. For a detailed explanation, see the training course IntroducingPVRTools
	//m_Print3D.DisplayDefaultTitle("Glass", "123", ePVRTPrint3DSDKLogo);
	//m_Print3D.Flush();

	return true;
}

/*!****************************************************************************
@Function		UpdateScene
@Description	Moves the scene.
******************************************************************************/
void OGLES2Glass::UpdateScene()
{
	// Fetch current time and make sure the previous time isn't greater
	unsigned long ulCurrentTime = PVRShellGetTime();
	if (ulCurrentTime < m_ulTime) { m_ulTime = ulCurrentTime; }

	// Calculate the time difference
	unsigned long ulTimeDifference = ulCurrentTime - m_ulTime;

	// Store the current time for the next frame
	m_ulTime = ulCurrentTime;

	m_afAngles[0] += ulTimeDifference * 0.002f;
	m_afAngles[1] -= ulTimeDifference * 0.0008f;

	float fRise = sin(m_afAngles[0] * 3.0f);

	// Rotate the camera
	//m_mView = PVRTMat4::LookAtRH(PVRTVec3(0, 0, -20), PVRTVec3(0, 0, 0), PVRTVec3(0, 1,
	//                             0)) * PVRTMat4::RotationY(m_afAngles[0] * 0.2f);


	// Rotate the balloon model matrices
	//m_mModels[0] = PVRTMat4::RotationY(m_afAngles[0]) * PVRTMat4::Translation(120.0f, fRise * 20.0f, 0.0f) * PVRTMat4::Scale(3.0f,
	//               3.0f, 3.0f);
	//m_mModels[1] = PVRTMat4::RotationY(m_afAngles[1]) * PVRTMat4::Translation(-180.0f, -fRise * 20.0f, 0.0f) * PVRTMat4::Scale(3.0f,
	//               3.0f, 3.0f);
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
void OGLES2Glass::DrawMesh(int i32NodeIndex, CPVRTModelPOD* pod, GLuint** ppuiVbos, GLuint** ppuiIbos, int i32NumAttributes)
{
	int i32MeshIndex = pod->pNode[i32NodeIndex].nIdx;
	SPODMesh* pMesh = &pod->pMesh[i32MeshIndex];

	// bind the VBO for the mesh
	glBindBuffer(GL_ARRAY_BUFFER, (*ppuiVbos)[i32MeshIndex]);
	// bind the index buffer, won't hurt if the handle is 0
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*ppuiIbos)[i32MeshIndex]);

	// Enable the vertex attribute arrays
	for (int i = 0; i < i32NumAttributes; ++i) { glEnableVertexAttribArray(i); }

	// Set the vertex attribute offsets
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sVertex.nStride, pMesh->sVertex.pData);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sNormals.nStride, pMesh->sNormals.pData);
	glVertexAttribPointer(TANGENT_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sTangents.nStride, pMesh->sTangents.pData);
	glVertexAttribPointer(BINORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, pMesh->sBinormals.nStride, pMesh->sBinormals.pData);
	if (pMesh->psUVW) {
		glVertexAttribPointer(TEXCOORD_ARRAY1, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[0].nStride, pMesh->psUVW[0].pData);
		//glVertexAttribPointer(TEXCOORD_ARRAY2, 2, GL_FLOAT, GL_FALSE, pMesh->psUVW[1].nStride, pMesh->psUVW[1].pData);
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
	for (int i = 0; i < i32NumAttributes; ++i) { glDisableVertexAttribArray(i); }

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



/*!****************************************************************************
@Function		DrawBall
@Description	Draws the reflective and refractive ball onto the screen.
******************************************************************************/
void OGLES2Glass::DrawBall(PVRTVec3 position, PVRTVec3 diffuseColor)
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


	glUniformMatrix4fv(m_BlinnPhongProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());


	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);
	glUniform3fv(m_BlinnPhongProgram.auiLoc[eEyePos], 1, &vEyePosModel.x);

	// Calculate and set the model space light direction
	PVRTVec3 vLightDir = mModel.inverse() * m_globalLightDir;
	vLightDir = vLightDir.normalize();
	glUniform3fv(m_BlinnPhongProgram.auiLoc[eLightDir], 1, vLightDir.ptr());

	glUniform3fv(m_BlinnPhongProgram.auiLoc[eDiffuseColor], 1, diffuseColor.ptr());

	// Now that the uniforms are set, call another function to actually draw the mesh
	DrawMesh(0, &m_Ball, &m_puiBallVbo, &m_puiBallIndexVbo, 2);
}

/*!****************************************************************************
@Function		DrawCube
@Description	Draws a simple Cube into the screen.
******************************************************************************/
void OGLES2Glass::DrawCube(){
	// Set model view projection matrix
	PVRTMat4 mModel, mModelView, mMVP;

	mModel = PVRTMat4::Scale(5.0f, 5.0f, 1.0f);
	//mModel = mModel * PVRTMat4::Translation()
	mModelView = m_mView * mModel;
	mMVP = m_mProjection * mModelView;

	// Use shader program
	glUseProgram(m_DefaultProgram.uiId);

	glUniformMatrix4fv(m_DefaultProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);
	glUniform3fv(m_DefaultProgram.auiLoc[eEyePos], 1, &vEyePosModel.x);

	// Calculate and set the model space light direction
	PVRTVec3 vLightDir = mModel.inverse() * PVRTVec4(19, 22, -50, 0);
	vLightDir = vLightDir.normalize();
	glUniform3fv(m_DefaultProgram.auiLoc[eLightDir], 1, vLightDir.ptr());

	// Calculate and set the model space eye position
	PVRTVec3 vEyePos = mModelView.inverse() * PVRTVec4(0.0f, 0.0f, 0.0f, 1.0f);
	glUniform3fv(m_DefaultProgram.auiLoc[eEyePos], 1, vEyePos.ptr());

	// Now that the uniforms are set, call another function to actually draw the mesh
	DrawMesh(0, &m_Cube_test, &m_puiCube_testVbo, &m_puiCube_testIndexVbo, 2);
}

/*!****************************************************************************
@Function		DrawPlane
@Description	Draws your hair into the screen.
******************************************************************************/
void OGLES2Glass::DrawHead(){
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiHeadDiffTex);

	PVRTMat4 mModel, mModelView, mMVP;

	mModel = PVRTMat4::Identity() * PVRTMat4::RotationY(PVRT_PI / 1.1f);;
	mModel *= PVRTMat4::RotationX(PVRT_PI / 2);
	mModel *= PVRTMat4::Scale(0.5, 0.5, 0.5);

	mModelView = m_mView * mModel;
	mMVP = m_mProjection * mModelView;

	// Use shader program
	glUseProgram(m_BlinnPhongProgram.uiId);

	glUniformMatrix4fv(m_BlinnPhongProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);
	glUniform3fv(m_BlinnPhongProgram.auiLoc[eEyePos], 1, &vEyePosModel.x);

	// Calculate and set the model space light direction
	PVRTVec3 vLightDir = mModel.inverse() * m_globalLightDir;
	vLightDir = vLightDir.normalize();
	glUniform3fv(m_BlinnPhongProgram.auiLoc[eLightDir], 1, vLightDir.ptr());

	// Now that the uniforms are set, call another function to actually draw the mesh
	DrawMesh(0, &m_HeadModel, &m_puiHeadVbo, &m_puiHeadIndexVbo, 5);
}

/*!****************************************************************************
@Function		DrawHair
@Description	Draws your hair into the screen.
******************************************************************************/
void OGLES2Glass::DrawHair(int pass){

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiHairDiffWhiteTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiHairNMTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_uiHairFlowT2Tex);

	PVRTMat4 mModel, mModelView, mMVP;

	mModel = PVRTMat4::Identity() * PVRTMat4::RotationY(PVRT_PI / 1.1f);;
	mModel *= PVRTMat4::RotationX(PVRT_PI / 2);
	mModel *= PVRTMat4::Scale(0.5, 0.5, 0.5);

	mModelView = m_mView * mModel;
	mMVP = m_mProjection * mModelView;

	// Use shader program
	glUseProgram(m_DefaultProgram.uiId);

	glUniformMatrix4fv(m_DefaultProgram.auiLoc[eMVPMatrix], 1, GL_FALSE, mMVP.ptr());

	glUniform1f(m_DefaultProgram.auiLoc[ePass], (GLfloat)pass);

	// Set eye position in model space
	PVRTVec4 vEyePosModel;
	vEyePosModel = mModelView.inverse() * PVRTVec4(0, 0, 0, 1);
	glUniform3fv(m_DefaultProgram.auiLoc[eEyePos], 1, &vEyePosModel.x);

	// Calculate and set the model space light direction
	PVRTVec3 vLightDir = mModel.inverse() * m_globalLightDir;
	vLightDir = vLightDir.normalize();
	glUniform3fv(m_DefaultProgram.auiLoc[eLightDir], 1, vLightDir.ptr());


	// Now that the uniforms are set, call another function to actually draw the mesh
	DrawMesh(0, &m_HairModel, &m_puiHairModelVbo, &m_puiHairModelIndexVbo, 5);
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
	return new OGLES2Glass;
}

/******************************************************************************
End of file (OGLES2Glass.cpp)
******************************************************************************/
