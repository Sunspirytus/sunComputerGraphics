// This file was created by Filewrap 1.2
// Little endian mode
// DO NOT EDIT

#include "../PVRTMemoryFileSystem.h"

// using 32 bit to guarantee alignment.
#ifndef A32BIT
 #define A32BIT static const unsigned int
#endif

// ******** Start: DefaultFragShader.fsh ********

// File data
static const char _DefaultFragShader_fsh[] = 
	"#version 100\n"
	"uniform sampler2D diffuseMap;\n"
	"uniform sampler2D normalMap;\n"
	"uniform sampler2D specFlowMap;\n"
	"uniform lowp float Pass;\n"
	"\n"
	"varying mediump vec3 NormalModel;\n"
	"varying mediump vec2 TexCoords1;\n"
	"varying mediump mat3 ModelToTangentMatrix; \n"
	"varying mediump vec3 LightDir_TangentSpace;\n"
	"varying mediump vec3 ViewDir_TangentSpace;\n"
	"\n"
	"\n"
	"const mediump vec3 _Spec1Color = vec3(1.04, 0.04, 0.04);\n"
	"const mediump float _Spec1EX = 72.0;\n"
	"const mediump float _Spec1Shift = 0.1;\n"
	"const mediump vec3 _Spec2Color = vec3(0.12, 1.12, 0.12);\n"
	"const mediump float _Spec2EX = 48.0;\n"
	"const mediump float _Spec2Shift = 0.2;\n"
	"\n"
	"const mediump vec3 _LightColor = vec3(1.0, 1.0, 1.0);\n"
	"const mediump float _SpecBlinnEX = 108.0;\n"
	"\n"
	"mediump float lerp(mediump float a, mediump float b, mediump float w) {\n"
	"  return a + w*(b-a);\n"
	"}\n"
	"\n"
	"mediump vec3 UnpackScaleNormal(mediump vec4 packedNormal){\n"
	"\tmediump vec3 normal;\n"
	"\tnormal.xyz = packedNormal.xyz * 2.0 - 1.0;\n"
	"\treturn normal;\n"
	"}\n"
	"\n"
	"void main()\n"
	"{\n"
	"\n"
	"\thighp vec2 texcoords1 = TexCoords1;\n"
	"\ttexcoords1.y = 1.0 - texcoords1.y;\n"
	"\t\n"
	"\thighp vec4 Albedo = texture2D(diffuseMap, texcoords1).rgba;\n"
	"\tif(Pass == 1.0){\n"
	"\t\tif(Albedo.a <0.8){\n"
	"\t\t\tdiscard;\n"
	"\t\t}\t\t\n"
	"\t}\t\n"
	"\t\n"
	"\thighp vec3 Normal_TangentSpace_normalized = normalize(UnpackScaleNormal(texture2D(normalMap, texcoords1)));\n"
	"\n"
	"\thighp vec3 tanFlow = texture2D(specFlowMap, texcoords1).rgb * 2.0 - 1.0;\n"
	"\thighp vec3 normalTangent = normalize(ModelToTangentMatrix * NormalModel);\n"
	"\thighp vec3 tanDir1 = normalize(tanFlow + normalTangent * _Spec1Shift);\n"
	"\thighp vec3 tanDir2 = normalize(tanFlow + normalTangent * _Spec2Shift);\n"
	"\thighp vec3 halfDirTangent = normalize(LightDir_TangentSpace + ViewDir_TangentSpace);\n"
	"\thighp float nh = max(dot(Normal_TangentSpace_normalized, halfDirTangent), 0.0);\n"
	"\thighp float nl = max(dot(Normal_TangentSpace_normalized, LightDir_TangentSpace), 0.0);\n"
	"\thighp float diffuseTerm = nl;\n"
	"\thighp float specularTerm = pow(nh , _SpecBlinnEX);\n"
	"\thighp float th1 = dot(tanDir1, halfDirTangent);\n"
	"\thighp float th2 = dot(tanDir2, halfDirTangent);\n"
	"\thighp vec3 spec1 =  _Spec1Color * pow(sqrt(1.0 - th1 * th1), _Spec1EX);\n"
	"\thighp vec3 spec2 =  _Spec2Color * pow(sqrt(1.0 - th2 * th2), _Spec2EX);\n"
	"\thighp vec3 specBlinn = clamp(0.0, 1.0, nl) * _LightColor * specularTerm;\n"
	"\thighp vec3 diffuseColor = Albedo.rgb * vec3(0.0, 0.0, 0.0);\n"
	"\thighp vec3 color = vec3(0.0, 0.0, 0.0);\n"
	"\tcolor += diffuseColor * diffuseTerm;\n"
	"\tcolor += spec1;\n"
	"\tcolor += spec2;\n"
	"\tcolor += specBlinn;\n"
	"\t\n"
	"\tgl_FragColor = vec4(color, Albedo.a);\n"
	"}";

// Register DefaultFragShader.fsh in memory file system at application startup time
static CPVRTMemoryFileSystem RegisterFile_DefaultFragShader_fsh("DefaultFragShader.fsh", _DefaultFragShader_fsh, 2444);

// ******** End: DefaultFragShader.fsh ********

