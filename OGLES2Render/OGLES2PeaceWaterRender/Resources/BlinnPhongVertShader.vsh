uniform highp mat4 MVPMatrix;
uniform mediump vec3 LightDirModel;
uniform mediump vec3 EyePosModel;

attribute highp vec3 inVertex;
attribute mediump vec3 inNormal;
attribute mediump vec3 inTangent;
attribute mediump vec3 inBiNormal;
attribute highp vec2 inTexCoords;

varying mediump vec3 lightDirInModel;
varying mediump vec3 viewDirInModel;
varying mediump vec3 normalInModel;
varying mediump vec2 uv;

varying mediump vec3 colorOut;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	lightDirInModel = LightDirModel;
	viewDirInModel = (EyePosModel - inVertex.xyz);
	normalInModel = inNormal;
	uv = inTexCoords;
	
	//colorOut = gl_Position.zzz / gl_Position.www;
}
