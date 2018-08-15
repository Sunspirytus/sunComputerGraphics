// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: DefaultVertShader.vsh ********

// File data
static const char _DefaultVertShader_vsh[] = 
	"#version 100\n"
	"uniform highp mat4 MVPMatrix;\n"
	"uniform mediump vec3 LightDir;\n"
	"uniform highp vec3 EyePos;\n"
	"\n"
	"attribute highp vec3 inVertex;\n"
	"attribute mediump vec3 inNormal;\n"
	"attribute mediump vec3 inTangent;\n"
	"attribute mediump vec3 inBiNormal;\n"
	"attribute highp vec2 inTexCoords;\n"
	"\n"
	"varying mediump vec3 NormalModel;\n"
	"varying mediump vec2 TexCoords1;\n"
	"varying mediump mat3 ModelToTangentMatrix; \n"
	"varying mediump vec3 LightDir_TangentSpace;\n"
	"varying mediump vec3 ViewDir_TangentSpace;\n"
	"\n"
	"varying mediump mat3 TangentToWorldMatrix;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\t// Transform position\n"
	"\tgl_Position = MVPMatrix * vec4(inVertex, 1.0);\n"
	"\n"
	"\t// Pass through texture coordinates\n"
	"\tTexCoords1 = inTexCoords;\n"
	"\t\n"
	"\t//lightDir\n"
	"\thighp vec3 LightDir_ModelSpace = normalize(LightDir);\n"
	"\t\n"
	"\t// viewDir\n"
	"\thighp vec3 ViewDir_ModelSpace = normalize(EyePos - inVertex.xyz);\n"
	"\t// normal\n"
	"\tNormalModel = normalize(inNormal);\n"
	"\thighp vec3 tangentModel = normalize(inTangent);\n"
	"\thighp vec3 binormalModel = normalize(inBiNormal);\n"
	"\n"
	"\t//highp vec3 binormalModel = cross(normalModel, tangentModel);\n"
	"\tModelToTangentMatrix = mat3(tangentModel.x, binormalModel.x, NormalModel.x,\n"
	"\t                           \ttangentModel.y, binormalModel.y, NormalModel.y,\n"
	"\t                            tangentModel.z, binormalModel.z, NormalModel.z);                    \n"
	"\t\n"
	"\tLightDir_TangentSpace = normalize(ModelToTangentMatrix * LightDir_ModelSpace);\n"
	"\tViewDir_TangentSpace = normalize(ModelToTangentMatrix * ViewDir_ModelSpace);  \n"
	"\t    \n"
	"}\n";

// Register DefaultVertShader.vsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_DefaultVertShader_vsh("DefaultVertShader.vsh", _DefaultVertShader_vsh, 1447);

// ******** End: DefaultVertShader.vsh ********

