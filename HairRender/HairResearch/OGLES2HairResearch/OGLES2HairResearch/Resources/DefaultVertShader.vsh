#version 100
uniform highp mat4 MVPMatrix;
uniform mediump vec3 LightDir;
uniform highp vec3 EyePos;

attribute highp vec3 inVertex;
attribute mediump vec3 inNormal;
attribute mediump vec3 inTangent;
attribute mediump vec3 inBiNormal;
attribute highp vec2 inTexCoords;

varying mediump vec3 NormalModel;
varying mediump vec2 TexCoords1;
varying mediump mat3 ModelToTangentMatrix; 
varying mediump vec3 LightDir_TangentSpace;
varying mediump vec3 ViewDir_TangentSpace;

varying mediump mat3 TangentToWorldMatrix;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Pass through texture coordinates
	TexCoords1 = inTexCoords;
	
	//lightDir
	highp vec3 LightDir_ModelSpace = normalize(LightDir);
	
	// viewDir
	highp vec3 ViewDir_ModelSpace = normalize(EyePos - inVertex.xyz);
	// normal
	NormalModel = normalize(inNormal);
	highp vec3 tangentModel = normalize(inTangent);
	highp vec3 binormalModel = normalize(inBiNormal);

	//highp vec3 binormalModel = cross(normalModel, tangentModel);
	ModelToTangentMatrix = mat3(tangentModel.x, binormalModel.x, NormalModel.x,
	                           	tangentModel.y, binormalModel.y, NormalModel.y,
	                            tangentModel.z, binormalModel.z, NormalModel.z);                    
	
	LightDir_TangentSpace = normalize(ModelToTangentMatrix * LightDir_ModelSpace);
	ViewDir_TangentSpace = normalize(ModelToTangentMatrix * ViewDir_ModelSpace);  
	    
}
