uniform highp mat4 MVPMatrix;
uniform mediump vec3 LightDir;
uniform mediump vec3 EyePos;

attribute highp vec3 inVertex;
attribute mediump vec3 inNormal;
attribute mediump vec3 inTangent;
attribute mediump vec3 inBiNormal;
attribute highp vec2 inTexCoords;

varying mediump vec3 lightDirInModel;
varying mediump vec3 viewDirInModel;
varying mediump vec3 normalInModel;
varying mediump vec2 uv;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	lightDirInModel = LightDir;
	viewDirInModel = (EyePos - inVertex.xyz);
	normalInModel = inNormal;
	uv = inTexCoords;
}
