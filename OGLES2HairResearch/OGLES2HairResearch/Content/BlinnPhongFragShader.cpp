// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: BlinnPhongFragShader.fsh ********

// File data
static const char _BlinnPhongFragShader_fsh[] = 
	"uniform mediump vec3 diffuseColor;\n"
	"uniform mediump sampler2D diffuseMap;\n"
	"\n"
	"varying mediump vec3 lightDirInModel;\n"
	"varying mediump vec3 viewDirInModel;\n"
	"varying mediump vec3 normalInModel;\n"
	"varying mediump vec2 uv;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tmediump vec3 lightDirInModel_normalized = normalize(lightDirInModel);\n"
	"\tmediump vec3 viewDirInModel_normalized = normalize(viewDirInModel);\n"
	"\tmediump vec3 normalInModel_normalized = normalize(normalInModel);\n"
	"\t\n"
	"\tmediump vec2 texCoord = uv;\n"
	"\ttexCoord.y = 1.0 - uv.y;\n"
	"\tmediump vec3 Albedo = texture2D(diffuseMap, texCoord).rgb;\n"
	"\t\n"
	"\tmediump vec3 diffuse = Albedo * max(dot(normalInModel_normalized, lightDirInModel_normalized), 0.0);\n"
	"\tmediump vec3 specularColor = vec3(0.5, 0.5, 0.5);\n"
	"\tmediump vec3 halfDir = normalize(lightDirInModel_normalized + viewDirInModel_normalized);\n"
	"\tmediump vec3 specular = specularColor * max(dot(normalInModel_normalized, halfDir), 0.0);\n"
	"\n"
	"\tgl_FragColor = vec4(diffuse + specular, 1.0);\n"
	"}";

// Register BlinnPhongFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_BlinnPhongFragShader_fsh("BlinnPhongFragShader.fsh", _BlinnPhongFragShader_fsh, 935);

// ******** End: BlinnPhongFragShader.fsh ********

