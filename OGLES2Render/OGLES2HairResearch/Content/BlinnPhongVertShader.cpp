// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BlinnPhongVertShader.vsh ********

// File data
static const char _BlinnPhongVertShader_vsh[] = 
	"uniform highp mat4 MVPMatrix;\n"
	"uniform mediump vec3 LightDir;\n"
	"uniform mediump vec3 EyePos;\n"
	"\n"
	"attribute highp vec3 inVertex;\n"
	"attribute mediump vec3 inNormal;\n"
	"attribute mediump vec3 inTangent;\n"
	"attribute mediump vec3 inBiNormal;\n"
	"attribute highp vec2 inTexCoords;\n"
	"\n"
	"varying mediump vec3 lightDirInModel;\n"
	"varying mediump vec3 viewDirInModel;\n"
	"varying mediump vec3 normalInModel;\n"
	"varying mediump vec2 uv;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\tlightDirInModel = LightDir;\n"
	"\tviewDirInModel = (EyePos - inVertex.xyz);\n"
	"\tnormalInModel = inNormal;\n"
	"\tuv = inTexCoords;\n"
	"}\n";

// Register BlinnPhongVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BlinnPhongVertShader_vsh("BlinnPhongVertShader.vsh", _BlinnPhongVertShader_vsh, 578);

// ******** End: BlinnPhongVertShader.vsh ********

